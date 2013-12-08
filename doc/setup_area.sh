# commands to set up a new dev area
#
# caveats:
# - if you don't have a github account, you need to change the git
#   protocol git: -> https: (see for example
#   http://stackoverflow.com/questions/7502247/is-an-ssh-key-required-to-clone-a-public-github-account)
# - the SusyNt patch might not be needed when using a recent
#   SusyNtuple version (try switching to the SusyNtuple trunk)

localSetupROOT # or asetup 17.3.4.6
SVN_INST=svn+ssh://svn.cern.ch/reps/atlasinst/Institutes
SVN_PHYS=svn+ssh://svn.cern.ch/reps/atlasphys/Physics
svn co ${SVN_INST}/UCIrvine/SUSYAnalysis/SusyNtuple/tags/SusyNtuple-00-01-04                    SusyNtuple
svn co ${SVN_PHYS}/SUSY/Analyses/WeakProduction/LeptonTruthTools/tags/LeptonTruthTools-00-01-06 LeptonTruthTools
svn co ${SVN_PHYS}/SUSY/Analyses/WeakProduction/MultiLep/tags/MultiLep-01-06-01                 MultiLep
svn co ${SVN_INST}/UCIrvine/mrelich/SusyXSReader/trunk                                          SusyXSReader
svn co ${SVN_PHYS}/SUSY/Analyses/WeakProduction/HistFitterTree/tags/HistFitterTree-00-00-15     HistFitterTree
git clone git@github.com:gerbaudo/SusyMatrixMethod.git
git clone git@github.com:gerbaudo/SusyTest0.git
cd SusyNtuple; patch <   ../SusyTest0/doc/SusyNtuple.patch2013-09-11 ; cd -
echo "check whether the patch was applied correctly..."
source MultiLep/installscripts/install_script.sh

./RootCore/scripts/find_packages.sh
./RootCore/scripts/build.sh