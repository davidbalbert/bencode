#include <stdlib.h>

#include "vec.h"

#define INITCAPA 5

typedef struct Vec {
    int len;
    int capa;
    void **data;
} Vec;

void *emalloc(size_t size);
void panic(char *s);

Vec *
vnew(void)
{
    Vec *v = emalloc(sizeof(Vec));
    v->capa = INITCAPA;
    v->len = 0;
    v->data = emalloc(INITCAPA * sizeof(void *));

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

void *
vget(Vec *v, int i)
{
    if (i >= v->len) {
        panic("vget: out of bounds");
    }

    return v->data[i];
}

void
veach(Vec *v, IterFunc f, void *extra)
{
    for (int i = 0; i < v->len; i++) {
        f(v->data[i], extra);
    }
}