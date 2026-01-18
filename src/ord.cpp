#include "ord.h"

#include <iostream>

namespace ord {

ordinal::ordinal (): terms (0) {}
ordinal::ordinal (size_t n) {
    if (n) {
        terms = std::vector<cterm> (1, {{zero, zero}, n});
    } else {
        terms = std::vector<cterm> (0);
    }
}

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

ordinal psi (const ordinal& id, const ordinal& v) {
    ordinal res;
    return res += id.tpsi (v);
}
ordinal psi (const ordinal& v) {
    ordinal res;
    return res += zero.tpsi (v);
}

ordinal::stdform ordinal::std () const { return ordinal::stdform (*this); }

size_t ordinal::complexity () const {
    size_t res = 0;
    for (const auto& [t, c] : terms) {
        const auto& [id, v] = t;
        res += std::max (id.complexity (), v.complexity ()) + c;
    }

    return res;
}

bool ordinal::to_next (size_t bound) {
    *this += one;
    while (complexity () > bound)
        if (!limit ()) return false;
    return true;
}

ordinal& ordinal::operator+= (const term& t) {
    while (terms.size () > 0 && terms.back ().t < t) terms.pop_back ();

    if (terms.size () > 0 && terms.back ().t == t) {
        ++terms.back ().c;
    } else {
        terms.emplace_back (t, 1);
    }

    return *this;
}
ordinal& ordinal::operator+= (term&& t) {
    while (terms.size () > 0 && terms[terms.size () - 1].t < t) terms.pop_back ();

    if (terms.size () > 0 && terms[terms.size () - 1].t == t) {
        ++terms[terms.size () - 1].c;
    } else {
        terms.emplace_back (std::move (t), 1);
    }

    return *this;
}

ordinal::term ordinal::tpsi (const ordinal& v) const {
    if (!v) return {*this, v};
    if (v.terms[0].t.id < *this) return {*this, v};

    auto bv = v.boost (v);
    if (!bv.has_value ()) return {*this + one, zero};

    return {*this, bv.value ()};
}

std::optional<ordinal> ordinal::boost (const ordinal& cv) const {
    ordinal res;

    for (const auto& [t, c] : terms) {
        const auto& [id, v] = t;

        auto obid = id.boost (cv);
        if (!obid.has_value ()) return {};
        auto& bid = obid.value ();

        if (bid >= cv) return {};
        if (bid > id) return res += bid.tpsi (zero);

        auto obv = v.boost (cv);
        if (!obv.has_value ()) return res += (id + one).tpsi (zero);
        auto& bv = obv.value ();

        if (bv >= cv) return res += (id + one).tpsi (zero);
        if (bv > v) return res += id.tpsi (bv);

        res.terms.emplace_back (t, c);
    }

    return res;
}

bool ordinal::limit () {
    if (terms.size ()) {
        auto [lt, lc] = std::move (terms.back ());
        terms.pop_back ();

        if (lc > 1) {
            lt.v += one;
            *this += std::move (lt);
        } else {
            if (lt.limit ()) {
                if (terms.size () && terms.back ().t <= lt) {
                    ++terms.back ().c;
                } else {
                    *this += lt;
                }
            } else {
                if (terms.size ()) {
                    ++terms.back ().c;
                } else {
                    return false;
                }
            }
        }

        return true;
    } else {
        return false;
    }
}

bool ordinal::term::limit () {
    if (v) {
        if (v.limit ()) {
            *this = id.tpsi (v);
        } else {
            id += one;
        }
    } else {
        return id.limit ();
    }

    return true;
}

std::strong_ordering ordinal::term::operator<=> (const ordinal::term& o) const {
    if (auto cmp = id <=> o.id; cmp != 0) {
        return cmp;
    } else {
        return v <=> o.v;
    }
}

const ordinal zero = ordinal ();
const ordinal one = psi (zero);
const ordinal omega = psi (one);
const ordinal Omega = psi (one, zero);

ordinal::stdform::stdform (): terms () {}

ordinal::stdform::stdform (ordinal::stdform::citerm&& cit): terms (1, std::move (cit)) {}

ordinal::stdform::operator bool () const { return !terms.empty (); }

bool ordinal::stdform::is_one () const { return terms.size () == 1 && !terms[0].it && terms[0].c == 1; }
bool ordinal::stdform::reduce_one () {
    if (terms.size () == 1 && !terms[0].it) {
        if (!--terms[0].c) {
            terms.pop_back ();
            return false;
        }
    }

    return true;
}

ordinal::stdform& ordinal::stdform::operator+= (ordinal::stdform&& sf) {
    while (terms.size () > 0 && terms.back ().it < sf.terms[0].it) terms.pop_back ();

    if (terms.size ()) {
        if (terms.back ().it == sf.terms[0].it) {
            terms.back ().c += sf.terms[0].c;
        } else {
            terms.push_back (std::move (sf.terms[0]));
        }
        for (size_t i = 1; i < sf.terms.size (); ++i) terms.push_back (std::move (sf.terms[i]));
    } else {
        *this = std::move (sf);
    }

    return *this;
}

ordinal::stdform::iterm ordinal::stdform::omega_to () {
    using iterm = ordinal::stdform::iterm;

    iterm res;
    for (auto& ct : terms) res *= ct.omega_to ();

    return res;
}

ordinal::stdform::stdform (const ordinal& o) {
    for (const auto& [t, c] : o.terms) {
        terms.emplace_back (iterm (t), c);
    }
}

std::ostream& operator<< (std::ostream& os, const ordinal::stdform& sf) {
    if (sf.terms.size ()) {
        size_t i = 0;
        for (const auto& t : sf.terms) {
            if (i++) os << '+';
            os << t;
        }
    } else {
        os << '0';
    }

    return os;
}
std::ostream& operator<< (std::ostream& os, const ordinal::stdform::stdterm& st) {
    using stdform = ordinal::stdform;
    const auto& [id, v] = st;

    if (!v) {
        os << "\\Omega";
        if (!id.is_one ()) os << "_{" << id << '}';
    } else {
        os << "\\psi";
        if (id) os << "_{" << id << '}';
        os << "\\left(" << v << "\\right)";
    }

    return os;
}
std::ostream& operator<< (std::ostream& os, const ordinal::stdform::iterm& it) {
    const auto& [mterms, oe] = it;

    for (const auto& mt : mterms) os << mt;

    if (oe) {
        os << "\\omega";
        if (!oe.is_one ()) os << "^{" << oe << '}';
    }

    if (!mterms.size () && !oe) os << '1';

    return os;
}
std::ostream& operator<< (std::ostream& os, const ordinal::stdform::citerm& cit) {
    const auto& [it, c] = cit;

    if (it) {
        os << it;
        if (c > 1) os << c;
    } else {
        os << c;
    }

    return os;
}
std::ostream& operator<< (std::ostream& os, const ordinal::stdform::mterm& mt) {
    using stdform = ordinal::stdform;
    const auto& [b, ix] = mt;

    os << b;
    if (!ix.is_one ()) os << "^{" << ix << '}';

    return os;
}

std::strong_ordering ordinal::stdform::operator<=> (const ordinal::stdform& o) const {
    for (size_t i = 0; i < std::min (terms.size (), o.terms.size ()); ++i) {
        if (auto cmp = terms[i] <=> o.terms[i]; cmp != 0) {
            return cmp;
        }
    }
    return terms.size () <=> o.terms.size ();
}

ordinal::stdform::iterm::iterm (): mterms (), oe () {}

ordinal::stdform::iterm::iterm (const term& t) {
    const auto& [id, v] = t;
    const auto& vterms = v.terms;

    ordinal prim, add;
    size_t i;
    for (i = 0; i < vterms.size () && vterms[i].t.id > id; ++i) prim.terms.push_back (vterms[i]);
    for (; i < vterms.size (); ++i) add.terms.push_back (vterms[i]);

    if (id || prim) mterms.emplace_back (stdterm{id.std (), prim.std ()}, stdform ({{}, 1}));

    *this *= add.std ().omega_to ();
}

std::strong_ordering ordinal::stdform::stdterm::operator<=> (const ordinal::stdform::stdterm& o) const {
    if (auto cmp = id <=> o.id; cmp != 0) {
        return cmp;
    }
    return v <=> o.v;
}

ordinal::stdform::iterm::operator bool () const { return mterms.size () || oe; }

ordinal::stdform::iterm& ordinal::stdform::iterm::operator*= (ordinal::stdform::iterm&& it) {
    if (it.mterms.size ()) {
        while (mterms.size () > 0 && mterms.back ().b < it.mterms[0].b) mterms.pop_back ();

        if (mterms.size ()) {
            if (mterms.back ().b == it.mterms[0].b) {
                mterms.back ().ix += std::move (it.mterms[0].ix);
            } else {
                mterms.push_back (std::move (it.mterms[0]));
            }
            for (size_t i = 1; i < it.mterms.size (); ++i) {
                mterms.push_back (std::move (it.mterms[i]));
            }

            oe = std::move (it.oe);
        } else {
            *this = std::move (it);
        }
    } else {
        oe += std::move (it.oe);
    }

    return *this;
}

std::strong_ordering ordinal::stdform::iterm::operator<=> (const ordinal::stdform::iterm& o) const {
    for (size_t i = 0; i < std::min (mterms.size (), o.mterms.size ()); ++i) {
        if (auto cmp = mterms[i] <=> o.mterms[i]; cmp != 0) {
            return cmp;
        }
    }
    if (auto cmp = mterms.size () <=> o.mterms.size (); cmp != 0) {
        return cmp;
    }
    return oe <=> o.oe;
}

ordinal::stdform::iterm ordinal::stdform::citerm::omega_to () {
    auto& [mterms, oe] = it;
    iterm res;

    if (mterms.size ()) {
        stdterm base = mterms[0].b;

        if (!mterms[0].ix.reduce_one ()) {
            mterms.erase (mterms.begin ());
        }

        res.mterms.emplace_back (std::move (base), stdform (std::move (*this)));
    } else {
        res.oe = stdform (std::move (*this));
    }

    return res;
}

std::strong_ordering ordinal::stdform::citerm::operator<=> (const ordinal::stdform::citerm& o) const {
    if (auto cmp = it <=> o.it; cmp != 0) {
        return cmp;
    }
    return c <=> o.c;
}

std::strong_ordering ordinal::stdform::mterm::operator<=> (const ordinal::stdform::mterm& o) const {
    if (auto cmp = b <=> o.b; cmp != 0) {
        return cmp;
    }
    return ix <=> o.ix;
}

}  // namespace ord
