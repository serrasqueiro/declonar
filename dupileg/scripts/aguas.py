# aguas.py  (c)2020  Henrique Moreira

"""
  Simple Excel readout

  Compatibility: python 3.
"""


from zexcess import *
from redito import xCharMap


#
# main()
#
def main (outFile, errFile, inArgs):
    debug = 0
    if inArgs==[]: return None
    cmd = inArgs[0]
    param = inArgs[1:]
    code = run_cmd(outFile, errFile, cmd, param, debug)
    return code


#
# run_cmd()
#
def run_cmd (outFile, errFile, cmd, param, debug=0):
    code = None
    adapt = dict()
    verbose = 0
    outName = None
    while len( param )>0 and param[0].startswith("-"):
        if param[ 0 ].startswith("-v"):
            verbose += param[ 0 ].count( "v" )
            del param[ 0 ]
            continue
        if param[ 0 ]=="--debug":
            debug = int( param[ 1 ] )
            del param[ :2 ]
            continue
        if param[ 0 ]=="--text":
            outName = param[ 1 ]
            del param[ :2 ]
            continue
        return None
    # Consolidate params
    # Do command...
    if cmd in ("show",
               "cat",
               ):
        if outName is not None:
            assert cmd=="cat"
            outFile = open(outName, "wb")
        sep = ";" if verbose<=0 else "\t"
        showOpts = (cmd, sep, adapt, verbose)
        code = show_table(outFile, param, showOpts, debug)
    if cmd in ("smas",):
        if outName is not None:
            assert cmd=="cat"
            outFile = open(outName, "wb")
        sep = "\t"
        adapt = {"*": {"replace": (("Data Inicial", "D.Inicio"),
                                   ("Data Final", "DataFinal"),
                                   )
                       },
                 "C": {"replace": (("Tipo de Consumo", "TipoC."),
                                   ("Estimado", "Est."),
                                   ),
                       },
                 "D": {"replace": (("Test:ABC", "Test:XYZ"),),
                       },
                 "D;E": {"replace": (("Consumido", "C."), # Consumido, e dias de consumo (column D and E)
                                     (" Consumo", ""),
                                   ),
                       },
                 }
        expand_adapt( adapt )
        if debug>0:
            for x, y in adapt.items():
                print("x:",x)
                print(y)
                print(".\n\n")
        showOpts = (cmd, sep, adapt, verbose)
        code = show_table(outFile, param, showOpts, debug)
    return code


#
# show_table
#
def show_table (outFile, param, showOpts, debug):
    code = 0
    if param==[]: return None
    cmd, sep, adapt, verbose = showOpts
    inName = param[0]
    rest = param[1:]
    z = ZSheets( inName, rest )
    sheetTrip, cont = z.sheets, z.cont
    idx = 0
    for pages in cont:
        idx += 1
        y = 0
        t = ZTable( pages )
        for entry in t.cont:
            y += 1
            aStr = t.alt_chr_separated( entry, adapt, sep )
            s = xCharMap.simpler_ascii( aStr )
            pre = "" if verbose<=0 else "row#{}\t".format( y )
            isBin = cmd=="cat"
            if isBin:
                outFile.write("{}{}\n".format( pre, s ).encode("ascii"))
            else:
                outFile.write("{}{}\n".format( pre, s ))
        shown = "{}, {}/ #{}".format( inName, idx, len(cont) )
        if debug>0:
            print("ZTable({}) minCol={}, maxCol={}".format( shown, t.minCol, t.maxCol ))
    return code


#
# Test suite
#
if __name__ == "__main__":
    import sys
    code = main( sys.stdout, sys.stderr, sys.argv[ 1: ] )
    if code is None:
        print("""Commands are:
show            Dump excel.
cat             Same as dump, but do not encode to ascii.
""")
        code = 0
    assert type( code )==int
    sys.exit( code )
