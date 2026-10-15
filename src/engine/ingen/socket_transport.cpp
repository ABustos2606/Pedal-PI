// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#include "engine/ingen/transport.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
using socket_t = SOCKET;
constexpr socket_t no_socket = INVALID_SOCKET;
constexpr int send_flags = 0;
#else
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using socket_t = int;
constexpr socket_t no_socket = -1;
// A vanished engine must be an error return, not a SIGPIPE that ends the interface.
constexpr int send_flags = MSG_NOSIGNAL;
#endif

namespace peepo::ingen {
namespace {
void close_socket(socket_t fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    ::close(fd);
#endif
}
void stop_socket(socket_t fd) {
#ifdef _WIN32
    shutdown(fd, SD_BOTH);
#else
    shutdown(fd, SHUT_RDWR);
#endif
}
void set_blocking(socket_t fd, bool blocking) {
#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    ioctlsocket(fd, FIONBIO, &mode);
#else
    const int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, blocking ? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
#endif
}
// The engine writes a reply in many small pieces. Over TCP each piece then
// waits for an acknowledgment this side would delay, which made every edit
// take about 50 ms; acknowledging at once brings it back to the socket's own
// speed. Unix sockets have no such delay, and ignore this.
void acknowledge_at_once(socket_t fd) {
#ifdef _WIN32
    int frequency = 1;
    DWORD returned = 0;
    WSAIoctl(fd, SIO_TCP_SET_ACK_FREQUENCY, &frequency, sizeof frequency, nullptr, 0, &returned, nullptr, nullptr);
#elif defined(TCP_QUICKACK)
    // Linux resets this by itself, so it is renewed after every read.
    const int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &on, sizeof on);
#else
    (void)fd;
#endif
}
void send_at_once(socket_t fd) {
    const int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&on), sizeof on);
}
bool in_progress() {
#ifdef _WIN32
    return WSAGetLastError() == WSAEWOULDBLOCK;
#else
    return errno == EINPROGRESS;
#endif
}
struct Link {
    std::mutex mutex;
    std::condition_variable wake;
    std::deque<std::string> outgoing;
    std::vector<Transport::Event> events;
    std::size_t buffered = 0;
    // closing: the owner let go. ended: the connection is over.
    bool closing = false, ended = false;
    socket_t fd = no_socket;
    std::thread reader;
};
// Wait in slices so that letting go of a link never waits for the network or
// for the other end: a blocked read is not reliably interrupted everywhere.
// Returns the events seen, 0 after `timeout` milliseconds, or -1 once closing.
int wait_for(Link& link, socket_t fd, short wanted, int timeout) {
    for (int waited = 0; waited < timeout; waited += 50) {
        { const std::lock_guard lock(link.mutex); if (link.closing) return -1; }
#ifdef _WIN32
        WSAPOLLFD item{fd, wanted, 0};
        const int ready = WSAPoll(&item, 1, 50);
#else
        pollfd item{fd, wanted, 0};
        const int ready = poll(&item, 1, 50);
#endif
        if (ready < 0) return -1;
        if (ready > 0) return item.revents ? item.revents : -1;
    }
    return 0;
}
bool finish_connect(Link& link, socket_t fd) {
    const int seen = wait_for(link, fd, POLLOUT, 3000);
    if (seen <= 0) return false;
    int error = 0;
    socklen_t size = sizeof error;
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &size) != 0 || error != 0) return false;
    return (seen & POLLOUT) != 0;
}
socket_t try_connect(Link& link, int family, const sockaddr* target, socklen_t size) {
    const socket_t fd = socket(family, SOCK_STREAM, 0);
    if (fd == no_socket) return no_socket;
    set_blocking(fd, false);
    if (connect(fd, target, size) == 0 || (in_progress() && finish_connect(link, fd))) {
        set_blocking(fd, true);
        if (family != AF_UNIX) { send_at_once(fd); acknowledge_at_once(fd); }
        return fd;
    }
    close_socket(fd);
    return no_socket;
}
socket_t connect_to(Link& link, const std::string& address, std::string& error) {
    if (address.starts_with("unix://")) {
#ifdef _WIN32
        error = "Unix sockets are not available here; use tcp://host:port";
        return no_socket;
#else
        sockaddr_un target{};
        const auto path = address.substr(7);
        if (path.empty() || path.size() >= sizeof target.sun_path) { error = "Invalid engine socket path"; return no_socket; }
        target.sun_family = AF_UNIX;
        std::memcpy(target.sun_path, path.c_str(), path.size() + 1);
        const auto fd = try_connect(link, AF_UNIX, reinterpret_cast<const sockaddr*>(&target), sizeof target);
        if (fd == no_socket) error = "Engine is not reachable at " + address;
        return fd;
#endif
    }
    const auto colon = address.rfind(':');
    if (!address.starts_with("tcp://") || colon == std::string::npos || colon < 7 || colon + 1 == address.size()) {
        error = "Engine address must be unix:///path or tcp://host:port";
        return no_socket;
    }
    addrinfo hints{}, *found = nullptr;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(address.substr(6, colon - 6).c_str(), address.substr(colon + 1).c_str(), &hints, &found) != 0 || !found) {
        error = "Engine host is unknown: " + address;
        return no_socket;
    }
    socket_t fd = no_socket;
    for (auto* candidate = found; candidate && fd == no_socket; candidate = candidate->ai_next)
        fd = try_connect(link, candidate->ai_family, candidate->ai_addr, static_cast<socklen_t>(candidate->ai_addrlen));
    freeaddrinfo(found);
    if (fd == no_socket) error = "Engine is not reachable at " + address;
    return fd;
}
void write_loop(const std::shared_ptr<Link>& link, socket_t fd) {
    for (;;) {
        std::string message;
        {
            std::unique_lock lock(link->mutex);
            link->wake.wait(lock, [&] { return !link->outgoing.empty() || link->closing || link->ended; });
            if (link->closing || link->ended) return;
            message = std::move(link->outgoing.front());
            link->outgoing.pop_front();
        }
        for (std::size_t sent = 0; sent < message.size();) {
            const auto count = send(fd, message.data() + sent, static_cast<int>(message.size() - sent), send_flags);
            // The reader sees the same failure and reports it.
            if (count <= 0) { stop_socket(fd); return; }
            sent += static_cast<std::size_t>(count);
        }
    }
}
void run(const std::shared_ptr<Link>& link, const std::string& address, std::size_t max_buffered) {
    std::string reason;
    const socket_t fd = connect_to(*link, address, reason);
    std::thread writer;
    {
        const std::lock_guard lock(link->mutex);
        if (fd != no_socket && !link->closing) {
            link->fd = fd;
            link->events.push_back({Transport::Event::Kind::Opened, {}});
            writer = std::thread(write_loop, link, fd);
        }
    }
    if (writer.joinable()) {
        reason = "Engine closed the connection";
        const bool tcp = !address.starts_with("unix://");
        std::string chunk(65536, '\0');
        for (;;) {
            const int seen = wait_for(*link, fd, POLLIN, 1000);
            if (seen < 0) break;
            if (seen == 0) continue;
            const auto count = recv(fd, chunk.data(), static_cast<int>(chunk.size()), 0);
            if (count <= 0) break;
            if (tcp) acknowledge_at_once(fd);
            const std::lock_guard lock(link->mutex);
            if (link->closing) break;
            if (!link->events.empty() && link->events.back().kind == Transport::Event::Kind::Data) link->events.back().data.append(chunk, 0, static_cast<std::size_t>(count));
            else link->events.push_back({Transport::Event::Kind::Data, chunk.substr(0, static_cast<std::size_t>(count))});
            link->buffered += static_cast<std::size_t>(count);
            if (link->buffered > max_buffered) { reason = "Interface fell too far behind the engine"; break; }
        }
    }
    {
        const std::lock_guard lock(link->mutex);
        link->ended = true;
        if (!link->closing) link->events.push_back({Transport::Event::Kind::Closed, reason});
    }
    link->wake.notify_all();
    if (fd != no_socket) stop_socket(fd);
    if (writer.joinable()) writer.join();
    {
        const std::lock_guard lock(link->mutex);
        link->fd = no_socket;
    }
    if (fd != no_socket) close_socket(fd);
}
class SocketTransport final : public Transport {
public:
    SocketTransport(std::string address, std::size_t max_buffered) : address_(std::move(address)), max_buffered_(max_buffered) {
#ifdef _WIN32
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
#endif
    }
    ~SocketTransport() override {
        close();
#ifdef _WIN32
        WSACleanup();
#endif
    }
    void open() override {
        close();
        link_ = std::make_shared<Link>();
        link_->reader = std::thread(run, link_, address_, max_buffered_);
    }
    void close() override {
        if (!link_) return;
        {
            const std::lock_guard lock(link_->mutex);
            link_->closing = true;
            link_->events.clear();
            if (link_->fd != no_socket) stop_socket(link_->fd);
        }
        link_->wake.notify_all();
        link_->reader.join();
        link_.reset();
    }
    void send(std::string message) override {
        if (!link_) return;
        {
            const std::lock_guard lock(link_->mutex);
            if (link_->ended) return;
            link_->outgoing.push_back(std::move(message));
        }
        link_->wake.notify_one();
    }
    std::vector<Event> poll() override {
        if (!link_) return {};
        const std::lock_guard lock(link_->mutex);
        link_->buffered = 0;
        auto events = std::move(link_->events);
        link_->events.clear();
        return events;
    }
private:
    std::string address_;
    std::size_t max_buffered_;
    std::shared_ptr<Link> link_;
};
}
std::unique_ptr<Transport> socket_transport(std::string address, std::size_t max_buffered) {
    return std::make_unique<SocketTransport>(std::move(address), max_buffered);
}
}
