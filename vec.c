#include <stdlib.h>

#include "vec.h"

#define VINITCAPA 5

typedef struct Vec Vec;
struct Vec {
    int len;
    int capa;
    void **data;
};

void *emalloc(size_t size);

Vec *
vnew(void)
{
    Vec *v = emalloc(sizeof(Vec));
    v->capa = VINITCAPA;
    v->len = 0;
    v->data = emalloc(VINITCAPA * sizeof(void *));

    return v;
}

void
vfree(Vec *v)
{
    free(v->data);
    free(v);
}

static void
vgrow(Vec *v)
{
    void **new = emalloc(2*v->capa * sizeof(void *));
    void **old = v->data;

    for (int i = 0; i < v->len; i++) {
        new[i] = old[i];
    }

    free(old);

    v->data = new;
    v->capa *= 2;
}

void
vappend(Vec *v, void *o)
{
    if (v->len == v->capa) {
        vgrow(v);
    }

    v->data[v->len++] = o;
}

void
veach(Vec *v, IterFunc f, void *extra)
{
    for (int i = 0; i < v->len; i++) {
        f(v->data[i], extra);
    }
}

void *
vfind(Vec *v, FinderFunc f, void *extra)
{
    for (int i = 0; i < v->len; i++) {
        if (f(v->data[i], extra)) {
            return v->data[i];
        }
    }

    return NULL;
}


static int
compare(void *thunk, const void *a, const void *b)
{
    SortFunc f = (SortFunc)thunk;

    void *ap = *((void **)a);
    void *bp = *((void **)b);

    return f(ap, bp);
}

void
vsort(Vec *v, SortFunc f)
{
    qsort_r(v->data, v->len, sizeof(void *), f, compare);
}
