# picsa_grep.py  (c)2019  Henrique Moreira (part of 'dupileg')

"""
  picsa_grep module handles picasa API xml atoms.

  Compatibility: python 2 and 3.
"""


import sys
#from os import fdopen


#
# test_picsa_grep -- Test module
#
def test_picsa_grep (out, args):
  params = args
  info = None
  verbose = 0
  output = out
  outName = None

  doAny = True
  while len( params )>0 and doAny:
    doAny = False
    if params[ 0 ].find( '-v' )==0:
      doAny = True
      verbose += params[ 0 ].count( 'v' )
      del params[ 0 ]
      info = sys.stderr
    if params[ 0 ]=='-o':
      doAny = True
      outName = params[ 1 ]
      del params[ :2 ]
      output = open( outName, "wb" )
  if len( params )>0:
    doAny = params[ 0 ].find( "-" )==0
    if doAny:
      show_usage()
  else:
    show_usage()
  # Main run:
  inputFileName = params[ 0 ]
  opts = params[ 1: ]
  doDecode = False
  if inputFileName!='.':
    fIn = lazy_open( inputFileName )
  else:
    fIn = sys.stdin
    #io.TextIOWrapper(sys.stdin.buffer, encoding='utf-8')
    doDecode = True

  flow = get_xml_atoms( fIn, verbose, info, doDecode )
  isOk = len( flow )>0
  if len( opts )>0:
    isOk = run_grep( sys.stdout, verbose, flow, opts )
  if outName:
    dump_content( output, flow )
  code = isOk==False
  return code


#
# dump_content() -- dump content linearly
#
def dump_content (output, flow):
  if sys.version_info.major>2:
    for r in flow:
      bLine = bytes( r+"\n", "latin1" )
      output.write( bLine )
  else:
    for r in flow:
      lat = r.decode( "utf-8" ).encode( "latin1" )
      bLine = lat+"\n"
      output.write( bLine )
  return True


#
# run_grep() -- interesting grep function over xml atoms
#
def run_grep (fOut, verbose, flow, opts, info=None):
  assert type( flow )==list
  assert type( opts )==list
  showAll = ("." in opts) or ("*" in opts)
  stt = 0
  chop = []
  for r in flow:
    isEntry = r.endswith( "entry>" )
    if isEntry:
      if r.find( "<entry>" )==0:
        stt = 1
        continue
      else:
        stt = 2
    if stt==2:
      if showAll:
        for aLine in chop:
          print( coarse_str( aLine ) )
        print( "<<<" )
        fe = FlowEntry( chop )
        if verbose > 0:
          fe.dump()
      else:
        fe = FlowEntry( chop )
        if fe.match_any_of( opts ):
          if verbose > 0:
            fe.dump()
          else:
            fOut.write( fe.x_str() + "\n" )
      chop = []
      stt = 0
    else:
      if stt==1:
        chop.append( r )
  return len( chop )<=0


#
# get_xml_atoms()
#
def get_xml_atoms (fIn, verbose, info=None, doDecode=False):
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
    uCode = ord( c )
    if uCode<127:
      trans = c
    else:
      trans = unicode_lazy( uCode )
    if trans!="":
      s = s + trans
    if trans=="":
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
        print(str( num )+": ", rStr)
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
        info.write(str( num )+": (!) " + dLine + "\n")
    if fails>0:
      info.write("Failed lines: " + str( fails ) + "\n")
  return flow


#
# show_usage()
def show_usage ():
  print("""picsa_grep.py [options] file.xml [name ...]

Options are:
	-v			Verbose.
""")
  sys.exit( 0 )



#
# lazy_open() -- works with Python2 and 3
#
def lazy_open (inName, encoding="utf-8"):
  if sys.version_info.major>2:
    fIn = open( inName, mode="r", encoding="utf-8" )
  else:
    fIn = open( inName, "r" )
  return fIn


#
# unicode_lazy() -- converts additional symbols from Unicode
#
def unicode_lazy (uCode, substDict=[]):
  if len( substDict )<=0:
    dSubst = { 0x25cf:"-o-", 0x1f463:"-y-" }
  else:
    dSubst = { substDict[ 0 ] }
  assert type( uCode )==int
  try:
    trans = dSubst[ uCode ]
  except:
    trans = ""
  return trans


#
# coarse_str() -- converts Latin1 or other chars into '.'
#
def coarse_str (inStr):
  assert type( inStr )==str
  s = ""
  for c in inStr:
    if ord( c )>=127:
      d = '.'
    else:
      d = c
    s += d
  return s


#
# CLASS FlowEntry
#
class FlowEntry:
  def __init__ (self, flow=[]):
    self.data_init()
    self.from_list( flow )


  def data_init (self):
    self.id = ""
    self.published = "YYYY-MM-DDThh:mm:ss.000Z"
    self.updated = "YYYY-MM-DDThh:mm:ss.000Z"
    self.scheme = "2005#kind"
    self.author = ("", "-")
    self.title = ""		# <title type='text'>\n$TITLE</title>
    self.linkRelFeed = ""	# <link rel='...' type='...atom+xml' href='https://.../feed/api/user/$PUID/albumid/$ALB_ID'/>
    self.hDict = {}


  def x_str (self, split=[" ;"]):
    s = ""
    fieldSplit = split[ 0 ]
    xName = self.title.replace( ";", "" )
    s += "TITLE: " + coarse_str( xName ) + fieldSplit
    s += "ALBUM: " + self.id
    return s


  def from_list (self, flow):
    assert type( flow )==list
    self.tuples = []
    self.bogus = []
    lastKey = ""
    groupHash = 0
    lines = flow
    # Hard-coded hashes for controlling groups:
    author = None
    authorURI = "-"
    preAuthor = self.hash_16bit( "<author>" )
    # ...and the remaining vars.
    aTitle = None
    dataFeedAPI = None
    # Iterate all lines:
    for aLine in lines:
      s = aLine.strip()
      single = s.endswith( "/>" )
      starts = s.find( "<" )==0
      if single:
        isOk = lastKey=="" and starts==True
        if not isOk:
          self.bogus.append( s )
        else:
          wKey = s[ :-2 ] + s[ -1 ]
          pos = wKey.find( " " )
          if pos<0:
            keyType = wKey
            wKey  = ""
          else:
            keyType = wKey[ 1:pos ].strip()
            wKey = wKey[ pos+1:-1 ].strip()
          self.tuples.append( (keyType, wKey, "@" ) )
          if keyType=="link":
            if wKey.find( "/data/feed/api/user/" )>=0:
              dataFeedAPI = wKey.split( "href=" )[ -1 ]
            pass
      else:
        if s.endswith( "author>" ) or s.endswith( "media:group>" ):
          gh = self.hash_16bit( s )
          self.hDict[ s ] = gh
          groupHash = gh
          continue
        if starts:
          if lastKey!="":
            self.bogus.append( [lastKey, "Another_key:"+s.split( " " )[0].strip("<")] )
          lastKey = s
        else:
          # Main tag: <abc>\nvalue</abc>
          juice = s.split( "<" )[ 0 ].strip()
          self.tuples.append( (lastKey, s, "h@"+str( groupHash )) )
          if groupHash==preAuthor:
            if lastKey=="<name>":
              author = juice
            else:
              authorURI = juice
          elif lastKey=="<id>":
            self.id = juice
          elif lastKey=="<title>" or lastKey=="<title type='text'>":
            if not aTitle:
              aTitle = juice
          else:
            pass
          lastKey = ""
      if author:
        self.author = (author, authorURI)
      if aTitle:
        self.title = aTitle
      if dataFeedAPI:
        self.linkRelFeed = dataFeedAPI
    return True


  def match_any_of (self, names):
    hit = self.title in names
    if hit:
      return True
    aTitle = coarse_str( self.title )
    for n in names:
      pos = aTitle.find( n )
      if pos==0:
        return True
    return False


  def dump (self):
    print("@id:", self.id)
    print("@author:", self.author)
    print("@title:", coarse_str( self.title ))
    print("@linkRelFeed:", self.linkRelFeed)
    print("[")
    for t in self.tuples:
      if type( t )==tuple:
        print("\t" + "{"+t[ 0 ]+"}", coarse_str( t[ 1 ] ), t[ 2 ])
      else:
        print("???")
    print("]")
    for key, hVal in self.hDict.items():
      print("Hash", '{:06d}d'.format( hVal ), key)
    if len( self.bogus )>0:
      for a in self.bogus:
        print("!!! Bogus:", a)


  def hash_16bit (self, s):
    nonNegativeK = ((sys.maxsize + 1) * 2)
    return 1 + (hash( s ) % nonNegativeK) % 999983


  pass


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
  # Run main program!
  args = sys.argv[ 1: ]
  if len( args )>0 and args[ 0 ]=="--help":
    show_usage()
  code = test_picsa_grep( out, args )
  sys.exit( code )

