
#ifndef KALI_NOOP_INCLUDED
#define KALI_NOOP_INCLUDED

// ............................................................................

struct NoOp
{
    #define T typename
    template <T A> void operator () (A) const {}
    template <T A, T B> void operator () (A, B) const {}
    template <T A, T B, T C> void operator () (A, B, C) const {}
    template <T A, T B, T C, T D> void operator () (A, B, C, D) const {}
    template <T A, T B, T C, T D, T E> void operator () (A, B, C, D, E) const {}
    template <T A, T B, T C, T D, T E, T F> void operator () (A, B, C, D, E, F) const {}
    template <T A, T B, T C, T D, T E, T F, T G> void operator () (A, B, C, D, E, F, G) const {}
    template <T A, T B, T C, T D, T E, T F, T G, T H> void operator () (A, B, C, D, E, F, G, H) const {}
    #undef T
};

// ............................................................................

#endif // ~ KALI_NOOP_INCLUDED
