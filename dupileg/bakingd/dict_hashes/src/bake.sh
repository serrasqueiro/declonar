#!/bin/sh

do_compile ()
{
 local A
 local B
 for A in *.c; do
	B=$(basename $A .c)
	gcc -Werror -Wall -o ${B}.o -c ${A}
	[ $? != 0 ] && return 1
 done
 if [ -f hashing_div ]; then
	echo "Did not overwrite: hashing_div"
 else
	gcc *.o -o hashing_div
 fi
 return 0
}



#
# Main
#
do_compile
RES=$?
if [ $RES != 0 ]; then
	echo "Bogus!"
	exit 1
fi
exit 0

