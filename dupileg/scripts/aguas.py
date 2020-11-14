# aguas.py  (c)2020  Henrique Moreira

"""
  Simple Excel readout

  Compatibility: python 3.
"""

# pylint: disable=invalid-name, unused-argument

import sys
from zexcess import expand_adapt, ZSheets, ZTable
from tconfig.yglob import LPath
from redito import xCharMap


def main():
    """ Main """
    code = main_run(sys.stdout, sys.stderr, sys.argv[1:])
    if code is None:
        print("""Commands are:
show            Dump excel.
cat             Same as dump, but do not encode to ascii.
smas            see README
stocks          see README
""")
    sys.exit(code if code else 0)


def main_run (outFile, errFile, inArgs):
    """
    Main function.
    :param outFile: stdout or other stream
    :param errFile: stderr or other (error stream)
    :param inArgs: input arguments
    :return: error-code
    """
    debug = 0
    if inArgs == []:
        return None
    cmd = inArgs[0]
    param = inArgs[1:]
    code = run_cmd(outFile, errFile, cmd, param, debug)
    return code


def run_cmd (outFile, errFile, cmd, param, debug=0):
    """
    Run command.
    """
    code = None
    verbose = 0
    outName = None
    while len( param ) > 0 and param[0].startswith("-"):
        if param[ 0 ].startswith("-v"):
            verbose += param[ 0 ].count( "v" )
            del param[ 0 ]
            continue
        if param[ 0 ] == "--debug":
            debug = int( param[ 1 ] )
            del param[ :2 ]
            continue
        if param[ 0 ] == "--text":
            outName = param[ 1 ]
            del param[ :2 ]
            continue
        return None
    opts = {"out-name": outName,
            "verbose": verbose,
            }
    # Do command...
    code = run_one_cmd(outFile, cmd, param, opts, debug)
    return code


def run_one_cmd(outFile, cmd, param, opts, debug):
    """
    Run one command.
    """
    code = None
    verbose = opts["verbose"]
    outName = opts["out-name"]
    adapt = dict()
    sep = ";" if verbose <= 0 else "\t"
    if cmd in ("show",
               "cat",
               ):
        if outName is not None:
            assert cmd == "cat"
            outFile = open(outName, "wb")
        showOpts = (cmd, sep, adapt, verbose)
        code = show_table(outFile, param, showOpts, debug)
        return code

    if cmd in ("smas",):
        if outName is not None:
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
                 # Consumido, e dias de consumo (column D and E)
                 "D;E": {"replace": (("Consumido", "C."),
                                     (" Consumo", ""),
                                     ),
                         },
                 }
        expand_adapt(adapt)
        if debug > 0:
            for x, y in adapt.items():
                print("x:", x)
                print(y)
                print(".\n\n")
        showOpts = (cmd, sep, adapt, verbose)
        code = show_table(outFile, param, showOpts, debug)
        return code

    if cmd == "stocks":
        showOpts = (cmd, sep, adapt, verbose)
        code = show_stocks(outFile, param, showOpts, debug)
    return code


def show_table (outFile, param, showOpts, debug=0):
    """
    Show table.
    :param outFile: output stream
    :param param: parameters
    :param showOpts: show options
    :param debug: whether debug is required
    :return: None, on parameter(s) fault, or an error-code
    """
    code = 0
    if param == []:
        return None
    cmd, sep, adapt, verbose = showOpts
    a_path = LPath(param[0])
    inName = a_path.to_os_path()
    assert inName is not None
    rest = param[1:]
    z = ZSheets(inName, rest)
    _, cont = z.sheets, z.cont
    idx = 0
    for pages in cont:
        idx += 1
        y = 0
        t = ZTable(pages)
        for entry in t.cont:
            y += 1
            aStr = t.alt_chr_separated( entry, adapt, sep )
            s = xCharMap.simpler_ascii( aStr )
            pre = "" if verbose <= 0 else "row#{}\t".format( y )
            isBin = cmd == "cat"
            if isBin:
                outFile.write("{}{}\n".format( pre, s ).encode("ascii"))
            else:
                outFile.write("{}{}\n".format( pre, s ))
        shown = "{}, {}/ #{}".format( inName, idx, len(cont) )
        if debug > 0:
            print("ZTable({}) minCol={}, maxCol={}".format( shown, t.minCol, t.maxCol ))
    return code


def show_stocks(outFile, param, showOpts, debug):
    """ Show stocks from Excel file """
    _, sep, adapt, verbose = showOpts
    a_path = LPath(param[0])
    inName = a_path.to_os_path()
    assert inName is not None
    z = ZSheets(inName)
    _, cont = z.sheets, z.cont
    idx = 0
    for pages in cont:
        idx += 1
        y = 0
        t = ZTable(pages)
        for entry in t.cont:
            y += 1
            aStr = t.alt_chr_separated(entry, adapt, sep)
            s = xCharMap.simpler_ascii(aStr)
            pre = "" if verbose <= 0 else "row#{}\t".format(y)
            outFile.write("{}{}\n".format(pre, s))
        shown = "{}, {}/ #{}".format(inName, idx, len(cont))
        if debug > 0:
            print("ZTable({}) minCol={}, maxCol={}".format(shown, t.minCol, t.maxCol))
    return 0


if __name__ == "__main__":
    main()
