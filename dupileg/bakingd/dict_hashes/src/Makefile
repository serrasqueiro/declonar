# hashing_div, and other binaries

CC=gcc
CC_OPT=-Werror -Wall
EXECNAME=hashing_div

CFLAGS=-g $(CC_BASIC)

OBJS=\
	hashing_div_lang.o \
	main.o

.PHONY: goal target clean clearall

goal: $(EXECNAME)

target: goal
	strip $(EXENAME)

hashing_div: $(OBJS)
	$(CC) $(OBJS) -o $(EXECNAME)


.c.o:
	@echo "Compiling $< : obj is $@"
	$(CC) $(CC_OPT) $(CFLAGS) -o $@ -c $<

clean:
	rm -f core $(OBJS) $(EXECNAME)

clearall: clean
	rm -f hashing

