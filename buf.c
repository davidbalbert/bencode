#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BINITCAPA 128

typedef struct Buf Buf;
struct Buf {
    int capa;
    int len;
    char *data;
};

void *emalloc(size_t size);

Buf *
bnew(void)
{
    Buf *b = emalloc(sizeof(Buf));
    b->capa = BINITCAPA;
    b->len = 0;
    b->data = emalloc(BINITCAPA * sizeof(char));

    return b;
}

void
bfree(Buf *b)
{
    free(b->data);
    free(b);
}

static void
bgrow(Buf *b, size_t newcapa)
{
    assert(b->capa < newcapa);

    char *new = emalloc(newcapa * sizeof(char));
    char *old = b->data;

    for (int i = 0; i < b->len; i++) {
        new[i] = old[i];
    }

    free(old);

    b->data = new;
    b->capa = newcapa;
}

void
bappend(Buf *b, char *s)
{
    size_t newlen = b->len + strlen(s);

    if (newlen >= b->capa) {
        bgrow(b, newlen);
    }

    for (int i = b->len; i < newlen; i++) {
        b->data[i] = *s;
        s++;
    }

    b->len = newlen;
}

void
bputc(Buf *b, char c)
{
    if (b->len == b->capa) {
        bgrow(b, 2*b->capa);
    }

    b->data[b->len++] = c;
}

void
bputint(Buf *b, int i)
{
    int nchar = snprintf(NULL, 0, "%d", i) + 1;
    char s[nchar];

    snprintf(s, nchar, "%d", i);

    bappend(b, s);
}

char *
bstr(Buf *b)
{
    char *s = emalloc((b->len + 1) * sizeof(char));
    int i;

    for (i = 0; i < b->len; i++) {
        s[i] = b->data[i];
    }
    s[i] = '\0';

    return s;
}
