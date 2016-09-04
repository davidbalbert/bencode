OBJS =\
	  bencode.o\
	  buf.o\
	  vec.o

CFLAGS = -MD

bencode: $(OBJS)
	$(CC) $(LDFLAGS) -o bencode $(OBJS)

-include *.d

.PHONY: clean
clean:
	rm -rf bencode *.dSYM *.o *.d
