#pragma once

#include <compare>
#include <optional>
#include <ostream>
#include <vector>

namespace ord {

class ordinal {
    class term;

    struct cterm;
    std::vector<cterm> terms;

 public:
    [[nodiscard]]
    ordinal ();

    ordinal (const ordinal&) = default;
    ordinal (ordinal&&) = default;

    ordinal& operator= (const ordinal&) = default;
    ordinal& operator= (ordinal&&) = default;

    [[nodiscard]]
    operator bool () const;

    [[nodiscard]]
    bool operator== (const ordinal&) const;
    [[nodiscard]]
    std::strong_ordering operator<=> (const ordinal&) const;

    [[nodiscard]]
    ordinal operator+ (const ordinal&) const;
    [[nodiscard]]
    ordinal operator+ (ordinal&&) const;
    ordinal& operator+= (const ordinal&);
    ordinal& operator+= (ordinal&&);

    friend std::ostream& operator<< (std::ostream&, const ordinal&);
    friend std::ostream& operator<< (std::ostream&, const term&);

    friend ordinal psi (const ordinal&, const ordinal&);

 private:
    ordinal& operator+= (const term&);
    ordinal& operator+= (term&&);

    [[nodiscard]]
    term tpsi (const ordinal&) const;
    [[nodiscard]]
    std::optional<ordinal> boost (const ordinal&) const;
};

struct ordinal::term {
    ordinal id, v;

 public:
    [[nodiscard]]
    bool operator== (const term&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const term&) const;
};

struct ordinal::cterm {
    term t;
    size_t c;

    [[nodiscard]]
    bool operator== (const cterm&) const = default;
};

extern const ordinal zero;
extern const ordinal one;
extern const ordinal omega;

[[nodiscard]]
ordinal psi (const ordinal&, const ordinal&);
[[nodiscard]]
ordinal psi (const ordinal&);

std::ostream& operator<< (std::ostream&, const ordinal&);
std::ostream& operator<< (std::ostream&, const ordinal::term&);

}  // namespace ord
