// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Pedal PI contributors
#pragma once
#include <map>
#include <string>
#include <string_view>
#include <vector>

// The Turtle subset Ingen writes on its socket: prefix directives, blank-node
// subjects, nested anonymous nodes and typed literals. A tokenizing reader, so
// a period or bracket inside a string never ends a statement.
namespace pedal::turtle {
struct Term {
    enum class Kind { Iri, Blank, Literal };
    Kind kind = Kind::Iri;
    // Absolute IRI, blank-node label or the literal's lexical form.
    std::string value;
    // Absolute datatype IRI; empty for a plain string.
    std::string datatype;
    bool operator==(const Term&) const = default;
};
struct Triple { Term subject, predicate, object; };
class Reader {
public:
    // Relative references resolve against `base`, which a @base directive
    // replaces; with none they are kept as written.
    explicit Reader(std::string base = {}) : base_(std::move(base)) {}
    // Prefixes persist between calls: the engine declares them once per
    // connection. Returns an empty string, or what was wrong; triples already
    // appended before an error belong to complete statements.
    std::string parse(std::string_view text, std::vector<Triple>& out);
private:
    std::map<std::string, std::string, std::less<>> prefixes_;
    std::string base_;
    unsigned long long anonymous_ = 0;
    friend struct Parser;
};
}
