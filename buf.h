typedef struct Buf Buf;

Buf *bnew(void);
void bfree(Buf *b);
void bappend(Buf *b, char *s);
void bputc(Buf *b, char c);
void bputint(Buf *b, int i);
char *bstr(Buf *b);
