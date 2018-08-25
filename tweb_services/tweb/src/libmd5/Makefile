CFLAGS=-Wall
LIBRARY=libmd5.a
OBJS=md5.o

RANLIB=$(CROSS_COMPILE)ranlib
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

CLEANFILES=$(OBJS) $(LIBRARY)

.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c

$(LIBRARY): $(OBJS)
	$(AR) cr $(LIBRARY) $?
	$(RANLIB) $@

clean:
	rm -f $(CLEANFILES)


