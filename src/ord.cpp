#include "ord.h"

namespace ord {

ordinal::ordinal (): terms (0) {}

ordinal::operator bool () const { return !terms.empty (); }

bool ordinal::operator== (const ordinal& o) const { return terms == o.terms; }

std::strong_ordering ordinal::operator<=> (const ordinal& o) const {
    auto l = terms.size ();
    auto ol = o.terms.size ();

    for (size_t i = 0; i < std::min (l, ol); ++i) {
        const auto& [t, c] = terms[i];
        const auto& [ot, oc] = o.terms[i];

        if (auto cmp = t <=> ot; cmp != 0) {
            return cmp;
        } else if (auto cmp = c <=> oc; cmp != 0) {
            return cmp;
        }
    }

    return l <=> ol;
}

ordinal ordinal::operator+ (const ordinal& o) const {
    if (!o) return *this;

    auto rtn = terms.size ();
    const auto& olt = o.terms[0].t;
    while (rtn > 0 && terms[rtn - 1].t < olt) --rtn;

    if (rtn == 0) return o;

    ordinal res;
    for (size_t i = 0; i < rtn - 1; ++i) {
        res.terms.push_back (terms[i]);
    }

    if (terms[rtn - 1].t == olt) {
        res.terms.emplace_back (o.terms[0].t, terms[rtn - 1].c + o.terms[0].c);
    } else {
        res.terms.push_back (terms[rtn - 1]);
        res.terms.push_back (o.terms[0]);
    }

    for (size_t i = 1; i < o.terms.size (); ++i) {
        res.terms.push_back (o.terms[i]);
    }

    return res;
}

ordinal ordinal::operator+ (ordinal&& o) const {
    if (!o) return *this;

    auto rtn = terms.size ();
    const auto& olt = o.terms[0].t;
    while (rtn > 0 && terms[rtn - 1].t < olt) --rtn;

    if (rtn == 0) return o;

    ordinal res;
    for (size_t i = 0; i < rtn - 1; ++i) {
        res.terms.push_back (terms[i]);
    }

    if (terms[rtn - 1].t == olt) {
        res.terms.emplace_back (std::move (o.terms[0].t), terms[rtn - 1].c + o.terms[0].c);
    } else {
        res.terms.push_back (terms[rtn - 1]);
        res.terms.push_back (std::move (o.terms[0]));
    }

    for (size_t i = 1; i < o.terms.size (); ++i) {
        res.terms.push_back (std::move (o.terms[i]));
    }

    return res;
}

ordinal& ordinal::operator+= (const ordinal& o) {
    if (!o) return *this;

    while (terms.size () && terms.back ().t < o.terms[0].t) terms.pop_back ();

    if (terms.size ()) {
        if (terms.back ().t == o.terms[0].t) {
            terms.back ().c += o.terms[0].c;
        } else {
            terms.push_back (o.terms[0]);
        }

        for (size_t i = 1; i < o.terms.size (); ++i) terms.push_back (o.terms[i]);
    } else {
        terms = o.terms;
    }

    return *this;
}

ordinal& ordinal::operator+= (ordinal&& o) {
    if (!o) return *this;

    while (terms.size () && terms.back ().t < o.terms[0].t) terms.pop_back ();

    if (terms.size ()) {
        if (terms.back ().t == o.terms[0].t) {
            terms.back ().c += o.terms[0].c;
        } else {
            terms.push_back (std::move (o.terms[0]));
        }

        for (size_t i = 1; i < o.terms.size (); ++i) terms.push_back (std::move (o.terms[i]));
    } else {
        terms = std::move (o.terms);
    }

    return *this;
}

ordinal psi (const ordinal& id, const ordinal& v) {
    // TODO(c_wInf):
    // check the validity of this combination

    ordinal res;
    res.terms.push_back ({{id, v}, 1});
    return res;
}

std::ostream& operator<< (std::ostream& os, const ordinal& o) {
    if (o.terms.size ()) {
        size_t i = 0;
        for (const auto& [t, c] : o.terms) {
            if (i++) os << '+';
            os << t;
            if (c > 1) os << c;
        }
    } else {
        os << '0';
    }

    return os;
}

std::ostream& operator<< (std::ostream& os, const ordinal::term& t) {
    os << 'p' << t.id << '(' << t.v << ')';
    return os;
}

std::strong_ordering ordinal::term::operator<=> (const ordinal::term& o) const {
    if (auto cmp = id <=> o.id; cmp != 0) {
        return cmp;
    } else {
        return v <=> o.v;
    }
}

const ordinal zero = ordinal ();
const ordinal one = psi (zero, zero);
const ordinal omega = psi (zero, one);

}  // namespace ord
