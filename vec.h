typedef struct Vec Vec;

typedef void (*IterFunc)(void *o, void *extra);

Vec *vnew(void);
void vfree(Vec *v);
void vappend(Vec *v, void *o);
void veach(Vec *v, IterFunc f, void *extra);
