#
# Makefile - htsix
#

.PHONY: goal clean clearall \
	target \
	end


CFLAGS=-Wall -Werror
CC_DEBUG=-g $(CC_BASIC)
CC_OPT=$(CC_DEBUG) $(CFLAGS)

OBJS=htsix.o \
	cbath.o


goal: htsix

target: goal
	strip htsix

clean:
	rm -f *.o

clearall: clean
	rm -f htsix


htsix: $(OBJS)
	gcc $(LINK_OPT) -o htsix $(OBJS)

htsix.o: cbath.o

.c.o:
	gcc $(CC_OPT) -o $@ -c $<


