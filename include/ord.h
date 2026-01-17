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
    [[nodiscard]]
    ordinal (size_t);  // NOLINT(runtime/explicit)

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
    friend ordinal psi (const ordinal&);

    class stdform;

    stdform std () const;

    [[nodiscard]]
    size_t complexity () const;
    bool to_next (size_t);

 private:
    ordinal& operator+= (const term&);
    ordinal& operator+= (term&&);

    [[nodiscard]]
    term tpsi (const ordinal&) const;
    [[nodiscard]]
    std::optional<ordinal> boost (const ordinal&) const;

    bool limit ();
};

struct ordinal::term {
    ordinal id, v;

    bool limit ();

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
extern const ordinal Omega;

class ordinal::stdform {
    class stdterm;
    class iterm;
    class citerm;
    class mterm;

    std::vector<citerm> terms;

    [[nodiscard]]
    stdform ();
    [[nodiscard]]
    explicit stdform (citerm&&);

    [[nodiscard]]
    operator bool () const;
    [[nodiscard]]
    bool is_one () const;
    bool reduce_one ();

    stdform& operator+= (stdform&&);
    [[nodiscard]]
    iterm omega_to ();

 public:
    [[nodiscard]]
    explicit stdform (const ordinal&);

    friend std::ostream& operator<< (std::ostream&, const stdform&);
    friend std::ostream& operator<< (std::ostream&, const stdterm&);
    friend std::ostream& operator<< (std::ostream&, const iterm&);
    friend std::ostream& operator<< (std::ostream&, const citerm&);
    friend std::ostream& operator<< (std::ostream&, const mterm&);

    [[nodiscard]]
    bool operator== (const stdform&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const stdform&) const;
};

struct ordinal::stdform::stdterm {
    stdform id, v;

    [[nodiscard]]
    bool operator== (const stdterm&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const stdterm&) const;
};

struct ordinal::stdform::iterm {
    std::vector<mterm> mterms;
    stdform oe;

    [[nodiscard]]
    iterm ();
    [[nodiscard]]
    explicit iterm (const term&);

    [[nodiscard]]
    operator bool () const;

    iterm& operator*= (iterm&&);

    [[nodiscard]]
    bool operator== (const iterm&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const iterm&) const;
};

struct ordinal::stdform::citerm {
    iterm it;
    size_t c;

    [[nodiscard]]
    iterm omega_to ();

    [[nodiscard]]
    bool operator== (const citerm&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const citerm&) const;
};

struct ordinal::stdform::mterm {
    stdterm b;
    stdform ix;

    [[nodiscard]]
    bool operator== (const mterm&) const = default;
    [[nodiscard]]
    std::strong_ordering operator<=> (const mterm&) const;
};

[[nodiscard]]
ordinal psi (const ordinal&, const ordinal&);
[[nodiscard]]
ordinal psi (const ordinal&);

std::ostream& operator<< (std::ostream&, const ordinal&);
std::ostream& operator<< (std::ostream&, const ordinal::term&);

std::ostream& operator<< (std::ostream&, const ordinal::stdform&);
std::ostream& operator<< (std::ostream&, const ordinal::stdform::stdterm&);
std::ostream& operator<< (std::ostream&, const ordinal::stdform::iterm&);
std::ostream& operator<< (std::ostream&, const ordinal::stdform::citerm&);
std::ostream& operator<< (std::ostream&, const ordinal::stdform::mterm&);

}  // namespace ord
