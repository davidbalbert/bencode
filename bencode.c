#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum BTag {
    TInt,
    TString,
    TList,
    TDict,
} BTag;

typedef struct BObject BObject;
typedef struct BList BList;
typedef struct BDict BDict;

#define INITCAPA 5
struct BList {
    int len;
    int capa;
    BObject **a;
};

struct BDict {
    int len;
    BObject *d;
};

struct BObject {
    BTag tag;
    union {
        int i;
        char *s;
        BList *l;
        BDict *d;
    } data;
};

void
panic(char *s)
{
    fprintf(stderr, "%s\n", s);
    exit(1);
}

void *
emalloc(size_t size)
{
    void *p = calloc(1, size);

    if (p == NULL) {
        panic("out of memory");
    }

    return p;
}

static BObject *
balloc(BTag tag)
{
    BObject *o = emalloc(sizeof(BObject));
    o->tag = tag;

    return o;
}

static void
blfree(BList *l)
{
    free(l->a);
    free(l);
}

static void
bdfree(BDict *d)
{
}

void
bfree(BObject *o)
{
    switch (o->tag) {
        case TList:
            blfree(o->data.l);
            break;
        case TDict:
            bdfree(o->data.d);
            break;
        case TInt:
        case TString:
            break;
        default:
            panic("bfree: unknown tag");
            break;
    }

    free(o);
}

static void
blgrow(BList *l)
{
    BObject **new = emalloc(2*l->capa * sizeof(BObject *));
    BObject **old = l->a;

    for (int i = 0; i < l->len; i++) {
        new[i] = old[i];
    }

    free(old);

    l->a = new;
    l->capa *= 2;
}

static BObject *
blalloc()
{
    BObject *o;
    BList *l = emalloc(sizeof(BList));

    l->capa = INITCAPA;
    l->len = 0;
    l->a = emalloc(INITCAPA * sizeof(BObject *));

    o = balloc(TList);
    o->data.l = l;

    return o;
}

void
blappend(BList *l, BObject *o){
    if (l->len == l->capa) {
        blgrow(l);
    }

    l->a[l->len++] = o;
}

BObject *
blget(BList *l, int i)
{
    if (i >= l->len) {
        panic("blget: out of bounds");
    }

    return l->a[i];
}

void
printtab(int ntab) {
    for (int i = 0; i < ntab; i++) {
        printf("  ");
    }
}

void
bprint0(BObject *o, int ntab)
{
    BList *l;

    switch (o->tag) {
        case TInt:
            printtab(ntab);
            printf("BInt %d\n", o->data.i);
            break;
        case TString:
            printtab(ntab);
            printf("BString %s\n", o->data.s);
            break;
        case TList:
            l = o->data.l;
            printtab(ntab);
            printf("BList\n");
            for (int i = 0; i < l->len; i++) {
                bprint0(blget(l, i), ntab+1);
            }
            break;
        case TDict:
            panic("no dicts yet");
            break;
        default:
            panic("unknown tag");
            break;
    }
}

static BObject *
decodeint(char *s, char **sp) {
    int neg = 0;
    int n = 0;
    BObject *o;

    if (*s == '-') {
        neg = 1;
        s++;
    }

    for (;;) {
        if (isdigit(*s)) {
            n *= 10;
            n += *s - '0';
        } else if (*s == 'e') {
            s++;
            break;
        } else {
            fprintf(stderr, "decodeint: unexpected character `%c'\n", *s);
            exit(1);
        }
        s++;
    }
    *sp = s;

    if (neg) {
        n *= -1;
    }

    o = emalloc(sizeof(BObject));
    o->tag = TInt;
    o->data.i = n;

    return o;
}

static BObject *
decodestring(char *s, char **sp)
{
    int len = 0;
    int i = 0;
    BObject *o;

    for (;;) {
        if (isdigit(*s)) {
            len *= 10;
            len += *s - '0';
        } else if (*s == ':') {
            s++;
            break;
        } else {
            fprintf(stderr, "decodestring: unexpected length character `%c'\n", *s);
        }
        s++;
    }

    o = balloc(TString);
    o->data.s = emalloc(len * sizeof(char));

    for (i = 0; i < len; i++) {
        o->data.s[i] = *s;
        s++;
    }
    *sp = s;

    o->data.s[i] = '\0';

    return o;
}

static BObject * bdecode0(char *s, char **sp);

static BObject *
decodelist(char *s, char **sp)
{
    BObject *o = blalloc();
    BList *l = o->data.l;

    while (*s != 'e') {
        blappend(l, bdecode0(s, &s));
    }

    s++; // skip 'e'
    *sp = s;

    return o;
}

void
bprint(BObject *o) {
    bprint0(o, 0);
}

char *
bencode(BObject *o)
{
    return "";
}

static BObject *
bdecode0(char *s, char **sp)
{
    BObject *o;

    if (*s == 'i') {
        o = decodeint(s + 1, &s);
    } else if (*s == 'l') {
        o = decodelist(s + 1, &s);
    } else if (isdigit(*s)) {
        o = decodestring(s, &s);
    } else {
        fprintf(stderr, "bdecode0: unknown tag `%c'\n", *s);
        exit(1);
    }

    if (sp) {
        *sp = s;
    }

    return o;
}

BObject *
bdecode(char *s)
{
    return bdecode0(s, NULL);
}

int
main(int argc, const char *argv[])
{
    bprint(bdecode("l5:helloi123e7:goodbyee"));
    return 0;
}
