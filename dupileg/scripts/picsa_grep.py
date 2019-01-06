# picsa_grep.py  (c)2019  Henrique Moreira (part of 'dupileg')

"""
  picsa_grep module handles picasa API xml atoms.

  Compatibility: python 2 and 3.
"""


import sys


#
# test_picsa_grep -- Test module
#
def test_picsa_grep (out, args):
  params = args
  info = None
  verbose = 0

  doAny = True
  while len( params )>0 and doAny:
    doAny = False
    if params[ 0 ].find( '-v' )==0:
      doAny = True
      verbose += params[ 0 ].count( 'v' )
      del params[ 0 ]
      info = sys.stderr
  if len( params )>0:
    doAny = params[ 0 ].find( "-" )==0
    if doAny:
      show_usage()
  else:
    show_usage()
  # Main run:
  inputFileName = params[ 0 ]
  opts = params[ 1: ]
  if inputFileName!='.':
    fIn = open( inputFileName, mode="r", encoding="utf-8" )
  else:
    fIn = sys.stdin

  code = run_grep( fIn, verbose, info )
  return code


#
# run_grep() -- interesting grep function over xml atoms
#
def run_grep (fIn, verbose, info=None, doDecode=False):
  if doDecode:
    data = fIn.read().decode( "utf-8" )
  else:
    data = fIn.read()
  flow = []
  s = ""
  for c in data:
    if c=='\r':
      continue
    if c=='\n':
      if s=="":
        continue	# ignore empty lines
      flow.append( s )
      s = ""
      continue
    s += c
    if c==">":
      flow.append( s )
      s = ""
  if info:
    num = 0
    fails = 0
    for r in flow:
      failed = None
      num += 1
      try:
        rStr = r.encode('latin-1', 'strict')
        print(str( num )+":", type( r ), rStr)
      except:
        failed = num
      if failed:
        fails += 1
        dLine = ""
        for c in r:
          if ord( c )>=127 or c<' ' or c=='[' or c==']':
            if ord( c )>=127:
              tic = '[0x{:x}={:03d}d]'.format( ord(c), ord(c) )
            else:
              tic = '[{:03d}d]'.format( ord(c) )
          else:
            tic = c
          dLine += tic
        print(str( num )+": (!) ", dLine)
    info.write("Failed lines: " + str( fails ) + "\n")
  return 0


#
# show_usage()
def show_usage ():
  print("""picsa_grep.py [options] file.xml [name ...]

Options are:
	-v			Verbose.
""")
  sys.exit( 0 )


#
# Test suite
#
if __name__ == "__main__":
  out = sys.stdout
  # Usage examples:
  #	python3 picsa_grep.py picasa_ggle.xml
  # ...verbose:
  #	python3 picsa_grep.py -v picasa_ggle.xml
  #
  args = sys.argv[ 1: ]
  if len( args )>0 and args[ 0 ]=="--help":
    show_usage()
  code = test_picsa_grep( out, args )
  sys.exit( code )

