#!/bin/sh

# uptime.cgi -- Henrique Moreira
# A simple script to show the uptime and all

htm_start ()
{
 echo "Content-type: text/html"
 echo
 htm_heading "Uptime.cgi, on `hostname` - $0"
}

htm_heading ()
{
 echo "<TITLE>$*</TITLE>"
}

#
# Aux functions
#
str_substitute_nils ()
{
 tr \\0 " "
}

show_uptime ()
{
 uptime
}


#
# Main script
#
htm_start

if [ "$*" = "" ]; then
	show_uptime
	exit 0
fi

# No other option, bail out
echo "<BR><BR>What: `date`"

