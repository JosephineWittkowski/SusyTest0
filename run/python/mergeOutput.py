#!/bin/env python

# Merge output root files from samples that belong to the same group
#
# davide.gerbaudo@gmail.com
# Aug 2013

import collections
import glob
import optparse
import os
import re
import subprocess
from datasets import datasets
from utils import getCommandOutput, guessLatestTagFromLatestRootFiles, guessMonthDayTagFromLastRootFile

usage="""%prog [options] dir
Merge root files from samples belonging to the same group.

Example:
.%prog -v -g ttbar out/susysel
"""
parser = optparse.OptionParser(usage=usage)
parser.add_option('-o', '--output', help='output directory; default <input>/merged/')
parser.add_option('-O', '--overwrite', action='store_true', help='overwrite output')
parser.add_option('-g', '--groupregexp', default='.*', help='only matching groups')
parser.add_option('-t', '--tag', help='production tag; by default the latest one')
parser.add_option('-v', '--verbose', action='store_true', help='print details')
parser.add_option('-d', "--debug", action='store_true', help='print even more details')
(options, args) = parser.parse_args()
if len(args) != 1 : parser.error("incorrect number of arguments")
inputdir      = args[0]
group_regexp  = options.groupregexp
verbose       = options.verbose
debug         = options.debug
overwrite     = options.overwrite
outdir        = options.output if options.output else inputdir+'/merged/'
tag           = (options.tag if options.tag
                 else guessLatestTagFromLatestRootFiles(inputdir, debug))
tag           = tag if tag else guessMonthDayTagFromLastRootFile(inputdir, debug)
if verbose :
    print "Options:"
    print '\n'.join(["%s : %s" % (o, eval(o))
                     for o in ['inputdir', 'outdir', 'group_regexp', 'tag', 'overwrite',
                               'verbose', 'debug',]])
if not os.path.isdir(outdir) :
    os.mkdir(outdir)
    if verbose : print "created directory '%s'"%outdir
allDatasets = [d for d in datasets if not d.placeholder]
filenamesByGroup = collections.defaultdict(list)
rootfiles = filter(os.path.isfile, glob.glob(inputdir + "*.root"))
rootfiles = [rf for rf in rootfiles if tag in rf]
for rf in rootfiles :
    dsname = os.path.basename(rf).replace('.root','').replace(tag,'')
    if debug : print "'%s' -> dataset '%s'"%(rf, dsname)
    dataset = next((d for d in allDatasets if d.name==dsname), None)
    if not dataset :
        print "warning, cannot identify dataset for '%s'"%rf
        continue
    group = dataset.group
    if dataset.isNotToBeMerged : continue
    if not re.search(group_regexp, group) : continue
    if not group : print "warning, invalid group '%s' for '%s'"%(group, rf)
    if verbose and group not in filenamesByGroup : print "adding group '%s'"%group
    filenamesByGroup[group].append(rf)
nGroupsToMerge = len(filenamesByGroup.keys())
groupCounter = 0
for group, files in filenamesByGroup.iteritems() :
    groupCounter += 1
    if verbose :
        print "[%d/%d] %s (%d files)"%(groupCounter, nGroupsToMerge, group, len(files))
    outfile = outdir+'/'+group+tag+'.root'
    if overwrite and os.path.isfile(outfile) : os.remove(outfile)
    if debug : print "hadd %s\n\t%s"%(outfile, '\n\t'.join(files))
    cmd = "hadd %s %s" % (outfile, ' '.join(files))
    out = getCommandOutput(cmd)
    success = out['returncode']==0
    if not success : print "'%s' failed..."%group
    if debug : print out['stderr']+'\n'+out['stdout']
