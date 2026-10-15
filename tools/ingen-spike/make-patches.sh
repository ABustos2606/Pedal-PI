#!/usr/bin/env bash
# One-off authoring helper: edit the Ingen checkout, then export the three
# patches that 01-build.sh applies. Kept so the patches can be regenerated.
set -euo pipefail
out="$(dirname "$(readlink -f "$0")")/patches"
cd /opt/peepo/src/ingen
git checkout -q -- src
python3 - <<'PY'
def edit(path, pairs):
    s = open(path).read()
    for a, b in pairs:
        assert s.count(a) == 1, (path, a[:50])
        s = s.replace(a, b)
    open(path, "w").write(s)

edit("src/server/SocketServer.hpp", [
("""#include <memory>
""", """#include <functional>
#include <memory>
#include <mutex>
#include <utility>
"""),
("""/** The server side of an Ingen socket connection. */
class SocketServer
{""", """/** A socket reader that reports hangup to its owner.

    SocketReader::on_hangup() is virtual, but SocketServer only owned a plain
    SocketReader, so its own on_hangup() was never called and a client that
    went away stayed registered (and written to) forever.
*/
class HangupReader : public SocketReader
{
public:
	HangupReader(World&                               world,
	             Interface&                           iface,
	             const std::shared_ptr<raul::Socket>& sock,
	             std::function<void()>                hangup)
		: SocketReader(world, iface, sock)
		, _hangup(std::move(hangup))
	{}

protected:
	void on_hangup() override { _hangup(); }

private:
	std::function<void()> _hangup;
};

/** The server side of an Ingen socket connection. */
class SocketServer
{"""),
("""		, _reader(new SocketReader(world, *_sink, sock))""",
 """		, _reader(new HangupReader(world, *_sink, sock, [this] { on_hangup(); }))"""),
("""		_sink->set_respondee(_writer);
		engine.register_client(_writer);
""", """		_sink->set_respondee(_writer);
		engine.register_client(_writer);

		const std::lock_guard<std::mutex> lock{_mutex};
		_ready = true;
		if (_hung_up) {
			_engine.unregister_client(_writer);
			_writer.reset();
		}
"""),
("""	void on_hangup() {
		_engine.unregister_client(_writer);
		_writer.reset();
	}""", """	void on_hangup() {
		// The reader thread starts inside the SocketReader constructor, so a
		// client that leaves at once can get here before _writer exists.
		const std::lock_guard<std::mutex> lock{_mutex};
		_hung_up = true;
		if (_ready && _writer) {
			_engine.unregister_client(_writer);
			_writer.reset();
		}
	}"""),
("""private:
	server::Engine&               _engine;
""", """private:
	server::Engine&               _engine;
	std::mutex                    _mutex;
	bool                          _ready{false};
	bool                          _hung_up{false};
"""),
])

edit("src/SocketReader.cpp", [
("""		_msg_node = nullptr;
	}

	// Lock RDF world
""", """		_msg_node = nullptr;
	}

	// A reset connection ends the loop through _socket_error without ever
	// reaching the poll() hangup branch; report that departure too.
	on_hangup();

	// Lock RDF world
"""),
])

edit("src/ingen/ingen.cpp", [
("""			std::this_thread::sleep_for(std::chrono::milliseconds(125));""",
 """			// Replies and notifications leave from this loop; 125 ms made every
			// acknowledgment that late.
			std::this_thread::sleep_for(std::chrono::milliseconds(5));"""),
])
PY
mkdir -p "$out"
git diff -- src/server/SocketServer.hpp src/SocketReader.cpp > "$out/0001-unregister-clients-on-hangup.patch"
git diff -- src/ingen/ingen.cpp > "$out/0002-post-process-every-5ms.patch"
# The third patch touches a file the first one already changed: diff it
# against the tree with the first two applied.
git add -A src
python3 - <<'PY'
path = "src/SocketReader.cpp"
s = open(path).read()
a = """	if (c < 0) {
		self->_socket_error = errno;
		return 0;
	}
"""
assert s.count(a) == 1
open(path, "w").write(s.replace(a, a + """
	// End of stream. A TCP client that closes its side leaves the socket
	// readable for ever, without the hangup poll() reports for a Unix socket,
	// and this reader then spun at full load. Treat it as the departure it is.
	if (c == 0) {
		self->_socket_error = ECONNRESET;
	}
"""))
PY
git diff -- src/SocketReader.cpp > "$out/0003-end-reader-at-end-of-stream.patch"
git reset -q
git checkout -q -- src
wc -l "$out"/*.patch
