#!/bin/env python

# a cutflow table with several possible representations
#
# davide.gerbaudo@gmail.com
# Aug 2013

import re

class CutflowTable :
    def __init__(self, samples=[], selections=[], countsSampleSel={},
                 isRawCount=False, selectionRegexp='.*'):
        self.samples         = samples
        self.selections      = selections
        self.countsSampleSel = countsSampleSel
        self.rawcnt          = isRawCount
        self.selRegexp       = selectionRegexp
    def latex(self, colWidth = 12) :
        rawcnt = self.rawcnt
        css = self.countsSampleSel
        samples, selections, selRegexp = self.samples, self.selections, self.selRegexp
        endrow = ' \\\\'
        fwidthField = '%'+str(colWidth)+'s'
        ncols = 1+len(self.samples)
        tablePreamble = ('% \usepackage{booktabs}\n'
                         +'% \usepackage{placeins}\n'
                         +'\\begin{table}[htbp] \n'
                         + '\\begin{center} \n'
                         + '\\begin{tabular}{l' + 'r'*(1-ncols) + '} \n'
                         + '\\toprule ')
        tableEpilogue = ('\\bottomrule \n'
                         +'\\end{tabular} \n'
                         +'\\caption{Add a caption here} \n'
                         +'\\end{center} \n'
                         +'\\end{table} \n'
                         +'\\FloatBarrier \n')
                         
        header = ' & '.join(fwidthField % t for t in ['selection']+samples) + endrow
        lines = [' & '.join([fwidthField % f
                             for f in [sel]+[("%.0f" if rawcnt else "%.1f") % c
                                             for c in [css[s][sel]
                                                       if s in css and sel in css[s] else None
                                                       for s in samples ]
                                             ]
                             ]) + endrow
                             for sel in selections
                             if re.match(selRegexp, sel.strip())
                             ]
        tex = '\n'.join([tablePreamble,
                         header,
                         '\\midrule']
                        +lines
                        +[tableEpilogue])
        return tex
