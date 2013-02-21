#!/bin/env python

# Plot all the histograms produced by SusyPlot
#
# davide.gerbaudo@gmail.com
# Jan 2013

import collections, optparse, sys, glob
import ROOT as r
r.PyConfig.IgnoreCommandLineOptions = True
r.gROOT.SetBatch(1)

from NavUtils import getAllHistoNames, HistoNameClassifier, organizeHistosByType, setHistoType, setHistoSample
from SampleUtils import colors, guessSampleFromFilename

#########
# default parameters [begin]
defaultTag      = 'Jan21_n0115'
defaultInputDir = './anaplots/merged'
defaultSigFile  = './anaplots/wA_noslep_WH_2Lep_3_Jan21_n0115.AnaHists.root'
defaultSigScale = 10.0
# default parameters [end]
#########

parser = optparse.OptionParser()
parser.add_option("-i", "--input-dir", dest="inputdir", default=defaultInputDir,
                  help="input directory (default '%s')" % defaultInputDir)
parser.add_option("-t", "--tag", dest="tag", default=defaultTag,
                  help="production tag (default '%s')" % defaultTag)
parser.add_option("-s", "--sig-file", dest="sigFname", default=defaultSigFile,
                  help="signal file (default %s)" % defaultSigFile)
parser.add_option("-S", "--sig-scale", dest="sigScale", default=defaultSigScale,
                  help="signal scale factor (default %.1f)" % defaultSigScale)
parser.add_option("-v", "--verbose", action="store_true", dest="verbose", default=False,
                  help="print more details about what is going on")
(options, args) = parser.parse_args()
inputDir        = options.inputdir
prodTag         = options.tag
signalFname     = options.sigFname
signalScale     = options.sigScale
verbose         = options.verbose

inputFileNames = glob.glob(inputDir+'/'+'*'+prodTag+'*.root') + glob.glob(signalFname)
print 'input files:\n'+'\n'.join(inputFileNames)
inputFiles = [r.TFile.Open(f) for f in inputFileNames]


histosByType = collections.defaultdict(list)
classifier = HistoNameClassifier()

for fname, infile in zip(inputFileNames, inputFiles) :
    print '-'*3 + fname + '-'*3
    samplename = guessSampleFromFilename(fname)
    histoNames = getAllHistoNames(inputFiles[0], onlyTH1=True) #[:10] # get only 10 histos for now
    histoNames = [h for h in histoNames if any([h.startswith(p) for p in ['sr6', 'sr7', 'sr8', 'sr9']])]
    histos = [infile.Get(hn) for hn in histoNames]
    for h in histos :
        setHistoType(h, classifier.histoType(h.GetName()))
        setHistoSample(h, samplename)
    organizeHistosByType(histosByType, histos)

def isSignal(sampleName) : return 'WH_' in sampleName

def plotHistos(histosDict={'ttbar':None, 'zjets':None},
                outdir='./plots', extensions=['png',], # 'eps'],
               verbose=False) :
    allHistosEmpty = all([h.GetEntries()==0 for h in histosDict.values()])
    if allHistosEmpty : return
    hnames = [h.GetName() for h in histosDict.values()]
    #if '_pt_' not in hnames[0] : return
    assert 1 == len(set(hnames)),"some histos have different names, something is wrong:\n%s"%str(set(hnames))
    hname = hnames[0]
    if verbose : print "got %d histos for '%s' (samples : %s)" % (len(histosDict), hname, str(histosDict.keys()))
    can = r.TCanvas('can_'+hname, hname, 800, 600)
    can.cd()
    stack = r.THStack('stack_'+hname,'')
    rMarg, lMarg, tMarg = can.GetRightMargin(), can.GetLeftMargin(), can.GetTopMargin()
    legWidth, legHeight = 0.325, 0.225
    leg = r.TLegend(1.0 - rMarg - legWidth, 1.0 - tMarg - legHeight, 1.0 - rMarg, 1.0 - tMarg)
    leg.SetBorderSize(1)
    leg.SetFillColor(0)
    leg.SetFillStyle(0)

    firstHisto = None
    for s in ['diboson', 'ttbar', 'zjets', 'multijets'] :
        if s not in histosDict : continue
        h = histosDict[s]
        if not firstHisto : firstHisto = h
        h.SetFillColor(colors[s])
        h.SetDrawOption('bar')
        stack.Add(h)
        leg.AddEntry(h, s+" (%.2f)"%h.Integral(), 'F')
    stack.Draw('hist')
    stack.GetXaxis().SetTitle(firstHisto.GetXaxis().GetTitle())
    stack.GetYaxis().SetTitle(firstHisto.GetYaxis().GetTitle())
    hTot = stack.GetHistogram()
    legOnLeftSide = hTot.GetMaximumBin() > 0.5*hTot.GetNbinsX()
    if legOnLeftSide :
        leg.SetX1(lMarg)
        leg.SetX2(lMarg+legWidth)

    signal = next((h for s,h in histosDict.iteritems() if isSignal(s)), None)
    if signal :
        signal.SetLineColor(r.kRed)
        signal.SetLineWidth(2*signal.GetLineWidth())
        signal.Scale(signalScale)
        signal.Draw('same')
        leg.AddEntry(signal, signal.sample+" (x%.1f, %.2f)"%(signalScale, signal.Integral()), 'L')
        if signal.GetMaximum() > stack.GetMaximum() : stack.SetMaximum(1.1*signal.GetMaximum())
    leg.Draw()
    channel, plotRegion = firstHisto.type.ch, firstHisto.type.pr
    def writeLabel(can, label, font='') :
        tex = r.TLatex(0.0, 0.0, '')
        tex.SetNDC()
        if font : tex.SetTextFont(font)
        tex.SetTextAlign(31)
        tex.DrawLatex(1.0-can.GetTopMargin(), 1.0-can.GetRightMargin(), label)
    writeLabel(can, channel+', '+plotRegion, firstHisto.GetTitleFont())
    can.Update()
    for ext in extensions : can.SaveAs(outdir+'/'+hname+'_lin'+'.'+ext)
    stack.SetMaximum(5.*stack.GetMaximum())
    stack.SetMinimum(0.25)
    can.SetLogy()
    for ext in extensions : can.SaveAs(outdir+'/'+hname+'_log'+'.'+ext)

for k,v in histosByType.iteritems() :
    plotHistos(histosDict=dict([(h.sample, h) for h in v]))
               
