CFLAGS = -g -DLong=int
SRCS = sim.c code1.c code2.c code3.c code4.c code5.c code6.c code7.c \
	code8.c code9.c help.c iobox.c scan.c run.c simops1.c simops2.c \
	strutils.c utils.c dasm.c curterm.c
OBJS = $(SRCS:.c=.o)

sim68k: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	$(RM) -f sim68k $(OBJS)
