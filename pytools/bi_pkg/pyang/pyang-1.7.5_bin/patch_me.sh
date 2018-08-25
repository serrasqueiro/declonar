#!/bin/sh
#
# patch_me -- (c)2018  Henrique Moreira

usage ()
{
 echo "$0

Patches the complete production set, in an orderly manner.
"
 exit 0
}


patch_me ()
{
 local INPUT=$1
 local BNAME
 local DNAME
 local P
 local RES
 # usually pyang-1.7.5.tar.xz
 [ "$1" = "" ] && return 2
 shift
 BNAME=$(basename $INPUT .xz)
 DNAME=$(basename $BNAME .tar)
 echo "Goal: $BNAME, at $DNAME/"
 # Check patches list:
 for P in $* ; do
	if [ ! -f $P ]; then
		echo "Cowardly exiting, cannot read: $P"
		exit 2
	fi
 done
 # If destination dir exists, complain and exit:
 if [ -d $DNAME ]; then
	echo "$DNAME/ already exists, quitting."
	return 0
 fi
 # Decompressing main source:
 if [ -f $BNAME ]; then
	RES=0
 else
	xz -d $INPUT
	RES=$?
 fi
 if [ $RES = 0 ]; then
	tar xf $BNAME
 else
	echo "Uops, failed decompressing: $INPUT"
	return 1
 fi
 RES=0
 for P in $* ; do
	echo "Patching with: $P"
	# Example:	patch -p0 < lint_simpler.patch
	patch -p0 < lint_simpler.patch
	[ $? != 0 ] && RES=1
 done
 if [ $RES = 0 ]; then
	echo "You can do:	rm -f $BNAME"
	echo "and:"
	echo "	cp -aR $DNAME/ /tmp/$DNAME/"
 fi
 return $RES
}


check_dependencies ()
{
 local FAIL=1
 local W
 which patch > /dev/null
 W=$?
 [ -f /usr/bin/patch -o $W = 0 ] && FAIL=0
 return $FAIL
}


#
# Main script
#
[ "$*" ] && usage

# Check dependencies...
check_dependencies
RES=$?
if [ $RES != 0 ]; then
	echo "Bailing out, missing 'patch'"
fi


for F in pyang-1.7.5*.xz ; do
	if [ ! -f $F ]; then
		echo "Skipped: $F"
		continue
	fi
	patch_me $F *.patch
	RES=$?
	if [ $RES != 0 ]; then
		echo "Patch was not smooth, error-code: $RES"
	fi
done

