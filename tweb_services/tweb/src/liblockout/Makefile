CFLAGS=-Wall -Werror -O0 -g
#-pedantic -ansi
# those flags cause warning about ftruncate ->
# ftruncate is posix compilant and not neccessarily ansi
LIBRARY=liblockout.a
OBJS=liblockout.o
LDFLAGS=-L.

CC=$(CROSS_COMPILE)gcc
RANLIB=$(CROSS_COMPILE)ranlib
AR=$(CROSS_COMPILE)ar

CLEAN_FILES=$(OBJS) $(LIBRARY)


.c.o:
	@rm -f $@
	$(CC) $(CFLAGS) -c $*.c


$(LIBRARY): $(OBJS)
	$(AR) cr $(LIBRARY) $?
	$(RANLIB) $@


clean:
	rm -f $(CLEAN_FILES)

