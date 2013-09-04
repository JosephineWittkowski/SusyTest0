#!/bin/env python

# produce a latex cutflow table from the histograms produced by SusyPlot
#
# davide.gerbaudo@gmail.com
# Jan 2013

import collections, optparse, sys, glob
import re
import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True
r.gROOT.SetBatch(1)

from NavUtils import getAllHistoNames, HistoNameClassifier, organizeHistosByType, HistoType, HistoNameClassifier, setHistoType, setHistoSample
from SampleUtils import guessGroupFromFilename, isBkgSample
from PickleUtils import dumpToPickle

#########
# default parameters [begin]
validChannels   = ['all', 'ee', 'em', 'mm']
defaultChannel  = validChannels[0]
defaultPickle   = 'counts.pkl'
defaultHisto    = 'onebin'
defaultRefSyst  = 'NOM'
# default parameters [end]
#########

usage="""Using the histograms produced by SusyPlot, print a latex cutflow table.

Examples:
> ./python/makeCutflowTable.py anaplots/merged/  -c all -b -d -s '^sr\d$' # background
> ./python/makeCutflowTable.py anaplots/ -c all  -s '^sr\d$'   # signals
"""
parser = optparse.OptionParser(usage=usage)
parser.add_option("-c", "--channel", default=defaultChannel, help="one of : %s"%str(validChannels))
parser.add_option("-p", "--pickle", default=defaultPickle, help="output (default %s)" % defaultPickle)
parser.add_option("-d", "--data", action='store_true', default=False, help="print data")
parser.add_option("-b", "--totbkg", action='store_true', default=False, help="print tot. bkg")
parser.add_option("-r", "--rawcounts", action='store_true', default=False, help="raw rather than weighted")
parser.add_option("-H", "--histo", default=defaultHisto, help="histo from which we count (default '%s')" % defaultHisto)
parser.add_option("-s", "--selregexp", default='.*', help="print only mathing selections (default '.*', any sel); example -s '^sr\d$', see http://www.debuggex.com/r/V82_pzhNDT0ukHMR/1")
parser.add_option("-S", "--syst", default=defaultRefSyst, help="systematic (default '%s')" % defaultRefSyst)
parser.add_option("-v", "--verbose", action="store_true", default=False, help="print stuff")
(options, args) = parser.parse_args()
channel         = options.channel
printData       = options.data
printTotBkg     = options.totbkg
rawcnt          = options.rawcounts
referenceHisto  = options.histo
referenceSyst   = options.syst
selRegexp       = options.selregexp
pickleFile      = options.pickle
verbose         = options.verbose

inputs, ext = args, '.root'
if len(inputs) < 1 : parser.error("provide at least one input")
inputFileNames = [f  for i in inputs for f in glob.glob(i if i.endswith(ext) else i+'/*'+ext)]
assert channel in validChannels,"Invalid channel %s (should be one of %s)" % (channel, str(validChannels))
inputFiles = [r.TFile.Open(f) for f in inputFileNames]

if verbose :
    print "Options:\n" \
          + '\n'.join(["%s : %s" % (o, eval(o))
                       for o in ['channel', 'printData', 'printTotBkg',
                                 'rawcnt', 'referenceHisto', 'referenceSyst','selRegexp',
                                 'pickleFile']])
    print 'Input files:\n'+'\n'.join(inputFileNames)

# navigate the files and collect the histos
refHistoType = HistoType(pr='', ch=channel, var=referenceHisto, syst=referenceSyst)
histoNameClassifier = HistoNameClassifier()
histosByType = collections.defaultdict(list)
classifier = HistoNameClassifier()

for fname, infile in zip(inputFileNames, inputFiles) :
    samplename = guessGroupFromFilename(fname)
    histoNames = [n for n in getAllHistoNames(infile, onlyTH1=True)
                  if refHistoType.matchAllAvailabeAttrs( histoNameClassifier.histoType( n ) )]
    histos = [infile.Get(hn) for hn in histoNames]
    for h in histos :
        setHistoType(h, classifier.histoType(h.GetName()))
        setHistoSample(h, samplename)
    organizeHistosByType(histosByType, histos)
refHistos = histosByType # already filtered histonames, all histosByType are refHistos
allSamples = sorted(list(set([h.sample for histos in refHistos.values() for h in histos])))
allSamples += ['totbkg'] if printTotBkg else []
allSelects = sorted(list(set([k.pr for k in histosByType.keys()])))
if verbose : print 'allSamples : ',allSamples
if verbose : print 'allSelects : ',allSelects

# get the counts (adding up what needs to be merged by samplename)
sampleCountsPerSel = dict() # counts[sample][sel]
countsSampleSel = dict([(s, collections.defaultdict(float)) for s in allSamples])
for t, histos in refHistos.iteritems() :
    if t.ch != channel : continue
    for h in histos :
        sample, sel = h.sample, h.type.pr
        cnt = h.GetEntries() if rawcnt else h.Integral()
        skipIt = not printData and sample=='data'
        countIt = not skipIt
        countsSampleSel[sample][sel] += cnt if countIt else 0.0
        if printTotBkg and isBkgSample(sample) :
            countsSampleSel['totbkg'][sel] += cnt

if pickleFile : dumpToPickle(pickleFile, countsSampleSel)

# print the table
endrow = ' \\\\'
colWidth = 12
fwidthField = '%'+str(colWidth)+'s'
tablePreamble = '% \usepackage{booktabs}\n' \
                +'% \usepackage{placeins}\n' \
                +'\\begin{table}[htbp] \n' \
                + '\\begin{center} \n' \
                + '\\begin{tabular}{l' + 'r'*len(allSamples) + '} \n' \
                + '\\toprule '
tableEpilogue = '\\bottomrule \n' \
                +'\\end{tabular} \n' \
                +'\\caption{Add a caption here} \n' \
                +'\\end{center} \n' \
                +'\\end{table} \n' \
                +'\\FloatBarrier \n'
header = ' & '.join([fwidthField % t for t in ['selection']+allSamples]) + endrow

print
print tablePreamble
print header
print '\\midrule'
for sel in allSelects : # should really use list comprehension
    if not re.match(selRegexp, sel.strip()) : continue
    counts = [countsSampleSel[sam][sel]
              if sam in countsSampleSel and sel in countsSampleSel[sam] else None \
              for sam in allSamples ]
    line = ' & '.join([fwidthField % f for f in [sel]+[("%.0f" if rawcnt else "%.1f") % c for c in counts]]) + endrow
    print line
print tableEpilogue

