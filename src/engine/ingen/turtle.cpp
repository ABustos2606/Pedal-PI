// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#include "engine/ingen/turtle.hpp"
#include <stdexcept>

namespace pedal::turtle {
namespace {
constexpr std::string_view rdf_type = "http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
constexpr std::string_view xsd = "http://www.w3.org/2001/XMLSchema#";
constexpr unsigned max_depth = 32;
struct Failure : std::runtime_error { using std::runtime_error::runtime_error; };
bool space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
bool digit(char c) { return c >= '0' && c <= '9'; }
bool name_end(char c) { return space(c) || c == ';' || c == ',' || c == '[' || c == ']' || c == '(' || c == ')' || c == '<' || c == '"' || c == '\'' || c == '#'; }
void append_utf8(std::string& out, unsigned long code) {
    if (code < 0x80) out += static_cast<char>(code);
    else if (code < 0x800) { out += static_cast<char>(0xc0 | (code >> 6)); out += static_cast<char>(0x80 | (code & 0x3f)); }
    else if (code < 0x10000) {
        out += static_cast<char>(0xe0 | (code >> 12)); out += static_cast<char>(0x80 | ((code >> 6) & 0x3f)); out += static_cast<char>(0x80 | (code & 0x3f));
    } else if (code < 0x110000) {
        out += static_cast<char>(0xf0 | (code >> 18)); out += static_cast<char>(0x80 | ((code >> 12) & 0x3f));
        out += static_cast<char>(0x80 | ((code >> 6) & 0x3f)); out += static_cast<char>(0x80 | (code & 0x3f));
    } else throw Failure("escape is not a Unicode code point");
}
}
struct Parser {
    Reader& reader;
    std::string_view text;
    std::vector<Triple>& out;
    std::size_t at = 0;

    bool done() const { return at >= text.size(); }
    char peek(std::size_t ahead = 0) const { return at + ahead < text.size() ? text[at + ahead] : '\0'; }
    bool starts(std::string_view s) const { return text.substr(at, s.size()) == s; }
    void skip() {
        while (!done()) {
            if (space(peek())) ++at;
            else if (peek() == '#') while (!done() && peek() != '\n') ++at;
            else break;
        }
    }
    void expect(char c, const char* what) {
        skip();
        if (peek() != c) throw Failure(what);
        ++at;
    }
    unsigned long hex(unsigned digits) {
        if (at + digits > text.size()) throw Failure("truncated escape");
        unsigned long code = 0;
        for (unsigned i = 0; i < digits; ++i) {
            const char c = text[at++];
            code = code * 16 + (digit(c) ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : throw Failure("invalid escape"));
        }
        return code;
    }
    // A relative reference against the base, as RFC 3986 does for the forms a
    // writer produces: the engine's serd (0.30, Debian 12) writes </main/m3>
    // for <ingen:/main/m3>, its base being <ingen:/>. Without a base, as for
    // a bundle's main.ttl, a relative reference is kept as written.
    std::string resolve(std::string ref) const {
        const auto& base = reader.base_;
        const auto scheme_end = ref.find_first_of(":/?#");
        if (base.empty() || (scheme_end != std::string::npos && ref[scheme_end] == ':')) return ref;
        const auto colon = base.find(':');
        const auto fragment = base.substr(0, base.find('#'));
        if (ref.empty()) return fragment;
        if (ref[0] == '#' || ref[0] == '?') return fragment.substr(0, ref[0] == '?' ? fragment.find('?') : std::string::npos) + ref;
        if (ref.starts_with("//")) return base.substr(0, colon + 1) + ref;
        // After the scheme, and the authority when there is one.
        auto root = colon + 1;
        if (base.compare(root, 2, "//") == 0) root = std::min(base.find('/', root + 2), base.size());
        if (ref[0] == '/') return base.substr(0, root) + ref;
        const auto path = fragment.substr(0, fragment.find('?'));
        const auto slash = path.rfind('/');
        auto merged = (slash == std::string::npos || slash < root ? base.substr(0, root) + "/" : path.substr(0, slash + 1)) + ref;
        for (std::size_t dot; (dot = merged.find("/./")) != std::string::npos;) merged.erase(dot, 2);
        return merged;
    }
    std::string iri_reference() {
        std::string value;
        for (++at; ; ) {
            if (done()) throw Failure("unterminated IRI");
            const char c = text[at++];
            if (c == '>') return resolve(value);
            if (c == '\\') {
                const char kind = done() ? '\0' : text[at++];
                if (kind == 'u') append_utf8(value, hex(4));
                else if (kind == 'U') append_utf8(value, hex(8));
                else throw Failure("invalid IRI escape");
            } else if (static_cast<unsigned char>(c) <= 0x20 || c == '<' || c == '"') throw Failure("invalid character in IRI");
            else value += c;
        }
    }
    std::string prefixed_name() {
        const auto colon = text.find(':', at);
        std::size_t end = at;
        while (end < text.size() && !name_end(text[end])) ++end;
        // A local name may contain periods, but not end with the statement's.
        while (end > at && text[end - 1] == '.') --end;
        if (colon == std::string_view::npos || colon >= end) throw Failure("expected an IRI or prefixed name");
        const auto it = reader.prefixes_.find(text.substr(at, colon - at));
        if (it == reader.prefixes_.end()) throw Failure("undeclared prefix");
        auto value = it->second;
        for (std::size_t i = colon + 1; i < end; ++i) {
            if (text[i] == '\\' && i + 1 < end) ++i;
            value += text[i];
        }
        at = end;
        return value;
    }
    Term iri() {
        skip();
        return {Term::Kind::Iri, peek() == '<' ? iri_reference() : prefixed_name(), {}};
    }
    Term string_literal() {
        const char quote = peek();
        const bool longer = peek(1) == quote && peek(2) == quote;
        at += longer ? 3 : 1;
        Term term{Term::Kind::Literal, {}, {}};
        for (;;) {
            if (done()) throw Failure("unterminated string");
            const char c = text[at++];
            if (c == quote) {
                if (!longer) break;
                if (peek() == quote && peek(1) == quote) { at += 2; break; }
                term.value += c;
            } else if (c == '\\') {
                if (done()) throw Failure("unterminated string");
                switch (text[at++]) {
                case 't': term.value += '\t'; break; case 'b': term.value += '\b'; break;
                case 'n': term.value += '\n'; break; case 'r': term.value += '\r'; break;
                case 'f': term.value += '\f'; break; case '"': term.value += '"'; break;
                case '\'': term.value += '\''; break; case '\\': term.value += '\\'; break;
                case 'u': append_utf8(term.value, hex(4)); break;
                case 'U': append_utf8(term.value, hex(8)); break;
                default: throw Failure("invalid string escape");
                }
            } else if (!longer && (c == '\n' || c == '\r')) throw Failure("line break in a short string");
            else term.value += c;
        }
        if (starts("^^")) { at += 2; term.datatype = iri().value; }
        else if (peek() == '@') { ++at; while (!done() && !name_end(peek()) && peek() != '.') ++at; }
        return term;
    }
    Term number() {
        const auto begin = at;
        bool fraction = false, exponent = false;
        if (peek() == '+' || peek() == '-') ++at;
        while (digit(peek())) ++at;
        if (peek() == '.' && digit(peek(1))) { fraction = true; ++at; while (digit(peek())) ++at; }
        if ((peek() == 'e' || peek() == 'E') && (digit(peek(1)) || ((peek(1) == '+' || peek(1) == '-') && digit(peek(2))))) {
            exponent = true; at += 2; while (digit(peek())) ++at;
        }
        return {Term::Kind::Literal, std::string(text.substr(begin, at - begin)),
                std::string(xsd) + (exponent ? "double" : fraction ? "decimal" : "integer")};
    }
    Term blank_label() {
        const auto begin = at += 2;
        while (!done() && !name_end(peek())) ++at;
        while (at > begin && text[at - 1] == '.') --at;
        if (at == begin) throw Failure("empty blank node label");
        return {Term::Kind::Blank, std::string(text.substr(begin, at - begin)), {}};
    }
    Term anonymous(unsigned depth) {
        if (depth >= max_depth) throw Failure("blank nodes are nested too deeply");
        ++at;
        // A label no parsed `_:name` can take, unique for the whole connection.
        Term node{Term::Kind::Blank, "[" + std::to_string(reader.anonymous_++) + "]", {}};
        skip();
        if (peek() != ']') predicate_objects(node, depth + 1);
        expect(']', "expected ']'");
        return node;
    }
    bool keyword(std::string_view word) const {
        return starts(word) && (at + word.size() >= text.size() || name_end(text[at + word.size()]) || text[at + word.size()] == '.');
    }
    Term object(unsigned depth) {
        skip();
        const char c = peek();
        if (c == '<') return {Term::Kind::Iri, iri_reference(), {}};
        if (c == '"' || c == '\'') return string_literal();
        if (c == '[') return anonymous(depth);
        if (c == '(') throw Failure("collections are not supported");
        if (c == '_' && peek(1) == ':') return blank_label();
        if (digit(c) || ((c == '+' || c == '-' || c == '.') && (digit(peek(1)) || (peek(1) == '.' && digit(peek(2)))))) return number();
        for (std::string_view word : {"true", "false"}) if (keyword(word)) {
            at += word.size();
            return {Term::Kind::Literal, std::string(word), std::string(xsd) + "boolean"};
        }
        return {Term::Kind::Iri, prefixed_name(), {}};
    }
    void predicate_objects(const Term& subject, unsigned depth) {
        for (;;) {
            skip();
            Term predicate;
            if (peek() == 'a' && (space(peek(1)) || peek(1) == '<' || peek(1) == '[')) { ++at; predicate = {Term::Kind::Iri, std::string(rdf_type), {}}; }
            else predicate = iri();
            do { out.push_back({subject, predicate, object(depth)}); skip(); } while (peek() == ',' && ++at);
            if (peek() != ';') return;
            // Empty entries after a semicolon are legal: `a b ; ; c d ; .`
            while (peek() == ';') { ++at; skip(); }
            if (peek() == '.' || peek() == ']' || done()) return;
        }
    }
    void directive() {
        const bool base = starts("@base");
        at += base ? 5 : 7;
        skip();
        std::string name;
        if (!base) {
            const auto colon = text.find(':', at);
            if (colon == std::string_view::npos) throw Failure("expected a prefix name");
            name = std::string(text.substr(at, colon - at));
            at = colon + 1;
            skip();
        }
        if (peek() != '<') throw Failure("expected an IRI in a directive");
        auto value = iri_reference();
        expect('.', "expected '.' after a directive");
        if (base) reader.base_ = std::move(value);
        else reader.prefixes_[name] = std::move(value);
    }
    void statement() {
        if (starts("@prefix") || starts("@base")) return directive();
        Term subject;
        bool properties_optional = false;
        if (peek() == '[') { subject = anonymous(0); properties_optional = true; }
        else if (peek() == '_' && peek(1) == ':') subject = blank_label();
        else subject = iri();
        skip();
        if (!(properties_optional && peek() == '.')) predicate_objects(subject, 0);
        expect('.', "expected '.' after a statement");
    }
};
std::string Reader::parse(std::string_view text, std::vector<Triple>& out) {
    Parser parser{*this, text, out};
    std::size_t complete = out.size();
    try {
        for (parser.skip(); !parser.done(); parser.skip()) { parser.statement(); complete = out.size(); }
    } catch (const Failure& failure) {
        out.resize(complete);
        return std::string(failure.what()) + " near byte " + std::to_string(parser.at);
    }
    return {};
}
}
