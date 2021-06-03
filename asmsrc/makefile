CFLAGS = -g -Wno-dangling-else
SRCS = codegen.c directiv.c error.c eval.c globals.c instlook.c listing.c \
	main.c movem.c object.c opparse.c symbol.c assemble.c build.c \
	insttabl.c
OBJS = $(SRCS:.c=.o)

asm68k: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) -f asm68k $(OBJS)
