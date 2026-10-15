// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Peepo contributors
#include "catalogue/json.hpp"
#include <charconv>
#include <cmath>
#include <stdexcept>

namespace peepo::json {
namespace {
constexpr unsigned max_depth = 32;
struct Failure : std::runtime_error { using std::runtime_error::runtime_error; };
struct Parser {
    std::string_view text;
    std::size_t at = 0;
    char peek() const { return at < text.size() ? text[at] : '\0'; }
    void skip() { while (at < text.size() && (text[at] == ' ' || text[at] == '\t' || text[at] == '\n' || text[at] == '\r')) ++at; }
    void expect(char c, const char* what) { skip(); if (peek() != c) throw Failure(what); ++at; }
    bool word(std::string_view w) { if (text.substr(at, w.size()) != w) return false; at += w.size(); return true; }
    unsigned hex() {
        if (at + 4 > text.size()) throw Failure("truncated escape");
        unsigned code = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = text[at++];
            code = code * 16 + (c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : throw Failure("invalid escape"));
        }
        return code;
    }
    static void utf8(std::string& out, unsigned long code) {
        if (code < 0x80) out += static_cast<char>(code);
        else if (code < 0x800) { out += static_cast<char>(0xc0 | (code >> 6)); out += static_cast<char>(0x80 | (code & 0x3f)); }
        else if (code < 0x10000) {
            out += static_cast<char>(0xe0 | (code >> 12)); out += static_cast<char>(0x80 | ((code >> 6) & 0x3f)); out += static_cast<char>(0x80 | (code & 0x3f));
        } else {
            out += static_cast<char>(0xf0 | (code >> 18)); out += static_cast<char>(0x80 | ((code >> 12) & 0x3f));
            out += static_cast<char>(0x80 | ((code >> 6) & 0x3f)); out += static_cast<char>(0x80 | (code & 0x3f));
        }
    }
    std::string string() {
        expect('"', "expected a string");
        std::string out;
        for (;;) {
            if (at >= text.size()) throw Failure("unterminated string");
            const char c = text[at++];
            if (c == '"') return out;
            if (static_cast<unsigned char>(c) < 0x20) throw Failure("control character in a string");
            if (c != '\\') { out += c; continue; }
            if (at >= text.size()) throw Failure("unterminated string");
            switch (text[at++]) {
            case '"': out += '"'; break; case '\\': out += '\\'; break; case '/': out += '/'; break;
            case 'b': out += '\b'; break; case 'f': out += '\f'; break; case 'n': out += '\n'; break;
            case 'r': out += '\r'; break; case 't': out += '\t'; break;
            case 'u': {
                unsigned long code = hex();
                // A pair of escapes for one character beyond the basic plane.
                if (code >= 0xd800 && code < 0xdc00 && word("\\u")) {
                    const unsigned low = hex();
                    if (low < 0xdc00 || low > 0xdfff) throw Failure("invalid surrogate pair");
                    code = 0x10000 + ((code - 0xd800) << 10) + (low - 0xdc00);
                } else if (code >= 0xd800 && code < 0xe000) throw Failure("invalid surrogate");
                utf8(out, code);
                break;
            }
            default: throw Failure("invalid string escape");
            }
        }
    }
    Value value(unsigned depth) {
        if (depth > max_depth) throw Failure("nested too deeply");
        skip();
        Value v;
        const char c = peek();
        if (c == '{') {
            ++at; v.kind = Value::Kind::Object; skip();
            if (peek() == '}') { ++at; return v; }
            for (;;) {
                skip();
                auto key = string();
                if (v.get(key)) throw Failure("repeated key");
                expect(':', "expected ':'");
                v.members.emplace_back(std::move(key), value(depth + 1));
                skip();
                if (peek() == ',') { ++at; continue; }
                expect('}', "expected ',' or '}'");
                return v;
            }
        }
        if (c == '[') {
            ++at; v.kind = Value::Kind::Array; skip();
            if (peek() == ']') { ++at; return v; }
            for (;;) {
                v.items.push_back(value(depth + 1));
                skip();
                if (peek() == ',') { ++at; continue; }
                expect(']', "expected ',' or ']'");
                return v;
            }
        }
        if (c == '"') { v.kind = Value::Kind::String; v.text = string(); return v; }
        if (word("true")) { v.kind = Value::Kind::Boolean; v.boolean = true; return v; }
        if (word("false")) { v.kind = Value::Kind::Boolean; return v; }
        if (word("null")) return v;
        const auto* begin = text.data() + at;
        const auto result = std::from_chars(begin, text.data() + text.size(), v.number);
        if (result.ec != std::errc{} || result.ptr == begin || !std::isfinite(v.number)) throw Failure("expected a value");
        at += static_cast<std::size_t>(result.ptr - begin);
        v.kind = Value::Kind::Number;
        return v;
    }
};
}
const Value* Value::get(std::string_view key) const {
    for (const auto& [name, value] : members) if (name == key) return &value;
    return nullptr;
}
std::string parse(std::string_view text, Value& out) {
    Parser parser{text};
    try {
        out = parser.value(0);
        parser.skip();
        if (parser.at != text.size()) throw Failure("text after the document");
    } catch (const Failure& failure) {
        return std::string(failure.what()) + " near byte " + std::to_string(parser.at);
    }
    return {};
}
std::string quote(std::string_view text) {
    std::string out = "\"";
    for (const char c : text) {
        switch (c) {
        case '"': out += "\\\""; break; case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break; case '\r': out += "\\r"; break; case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                static const char digits[] = "0123456789abcdef";
                out += "\\u00"; out += digits[(c >> 4) & 15]; out += digits[c & 15];
            } else out += c;
        }
    }
    return out + "\"";
}
std::string number(double value) {
    char buffer[32];
    const auto result = std::to_chars(buffer, buffer + sizeof buffer, value);
    return std::string(buffer, result.ptr);
}
}
