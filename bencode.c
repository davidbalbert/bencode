#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.h"

typedef enum BTag {
    TInt,
    TString,
    TList,
    TDict,
} BTag;

typedef struct BObject BObject;

struct BObject {
    BTag tag;
    union {
        int i;
        char *s;
        Vec *v; // for TList and TDict
    } data;
};

typedef struct Pair {
    BObject *key;
    BObject *val;
} Pair;

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

void bfree(BObject *o);

static void
freelist(void *o, void *extra)
{
    bfree((BObject *)o);
}

static void
freedict(void *o, void *extra)
{
    Pair *p = (Pair *)o;

    bfree(p->key);
    bfree(p->val);

    free(p);
}

void
bfree(BObject *o)
{
    Vec *v;

    switch (o->tag) {
        case TList:
            v = o->data.v;
            veach(v, freelist, NULL);
            break;
        case TDict:
            v = o->data.v;
            veach(v, freedict, NULL);
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

static BObject *
listnew()
{
    BObject *o = balloc(TList);
    o->data.v = vnew();

    return o;
}

static void
listappend(BObject *l, BObject *o){
    assert(l->tag == TList);

    Vec *v = l->data.v;

    vappend(v, o);
}

static BObject *
dictnew()
{
    BObject *o = balloc(TDict);
    o->data.v = vnew();

    return o;
}

int
findpair(void *o, void *extra)
{
    Pair *pair = (Pair *)o;
    BObject *needle = (BObject *)extra;

    assert(pair->key->tag == TString);
    assert(needle->tag == TString);

    char *s1 = pair->key->data.s;
    char *s2 = needle->data.s;

    return strcmp(s1, s2) == 0;
}

static void
dictset(BObject *dict, BObject *key, BObject *val)
{
    assert(dict->tag == TDict);
    assert(key->tag == TString);

    Pair *pair = (Pair *)vfind(dict->data.v, findpair, key);

    if (pair) {
        bfree(pair->key);
        bfree(pair->val);

        pair->key = key;
        pair->val = val;
    } else {
        pair = emalloc(sizeof(Pair));

        pair->key = key;
        pair->val = val;

        vappend(dict->data.v, pair);
    }
}

static void
printtab(int ntab) {
    for (int i = 0; i < ntab; i++) {
        printf("  ");
    }
}

static void bprint0(BObject *o, int ntab);

static void
printlist(void *o, void *extra)
{
    int ntab = *((int *)extra);
    bprint0((BObject *)o, ntab+1);
}

static void
printdict(void *o, void *extra)
{
    int ntab = *((int *)extra);
    Pair *p = (Pair *)o;

    bprint0(p->key, ntab+1);
    bprint0(p->val, ntab+2);
}

static void
bprint0(BObject *o, int ntab)
{
    Vec *v;

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
            printtab(ntab);
            printf("BList\n");

            v = o->data.v;
            veach(v, printlist, &ntab);
            break;
        case TDict:
            printtab(ntab);
            printf("BDict\n");

            v = o->data.v;
            veach(v, printdict, &ntab);
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

    o = balloc(TInt);
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
    BObject *l = listnew();

    while (*s != 'e') {
        listappend(l, bdecode0(s, &s));
    }

    s++; // skip 'e'
    *sp = s;

    return l;
}

static BObject *
decodedict(char *s, char **sp)
{
    BObject *d = dictnew();
    BObject *k, *v;

    while (*s != 'e') {
        if (!isdigit(*s)) {
            panic("dict keys must be strings");
            exit(1);
        }

        k = decodestring(s, &s);
        v = bdecode0(s, &s);

        dictset(d, k, v);
    }

    s++; // skip 'e'
    *sp = s;

    return d;
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
    } else if (*s == 'd') {
        o = decodedict(s + 1, &s);
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
    bprint(bdecode("d1:ai1e1:ai2ee"));
    return 0;
}
