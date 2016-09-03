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

struct BList {
    int len;
    BObject *a;
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

BObject *
blget(BList *l, int i)
{
    if (i >= l->len) {
        panic("blget: out of bounds");
    }

    return &l->a[i];
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
decodeint(char *s) {
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
            break;
        } else {
            fprintf(stderr, "decodeint: unexpected character `%c'\n", *s);
            exit(1);
        }
        s++;
    }

    if (neg) {
        n *= -1;
    }

    o = emalloc(sizeof(BObject));
    o->tag = TInt;
    o->data.i = n;

    return o;
}

static BObject *
decodestring(char *s)
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

    o = emalloc(sizeof(BObject));
    o->tag = TString;
    o->data.s = emalloc(len * sizeof(char));

    for (i = 0; i < len; i++) {
        o->data.s[i] = s[i];
    }

    o->data.s[i] = '\0';

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

BObject *
bdecode(char *s)
{
    if (*s == 'i') {
        return decodeint(s + 1);
    } else if (isdigit(*s)) {
        return decodestring(s);
    } else {
        fprintf(stderr, "bdecode: unknown tag `%c'\n", *s);
        exit(1);
    }
}



int
main(int argc, const char *argv[])
{
    bprint(bdecode("5:hello"));
    return 0;
}
