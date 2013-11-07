// emacs -*- C++ -*-
#ifndef SUSY_WH_TUPLEMAKER_H
#define SUSY_WH_TUPLEMAKER_H

#include "SusyTest0/TupleMakerObjects.h"

#include <string>
#include <vector>

class TTree;
class TFile;
namespace Susy
{
class Lepton;
class Jet;
}
// LeptonVector is defined in SusyDefs.h, but that's a huge include just for one def...refactor
typedef std::vector<Susy::Lepton*> LeptonVector;
typedef std::vector<Susy::Jet*>    JetVector;

namespace susy
{
namespace wh
{
/*!
  A class to save the information from SusyNt to a simpler ntuple.
  
  Details:
  This class is meant to create small ntupled for faster turnaround.
  The nutples store the information relative to the following objects:
  - two leading leptons
  - met
  - jets
  - other leptons
  - event variables
  The information is converted from SusyNt classes to smaller and
  simpler objects (see TupleMakerObjects.h)
  
  davide.gerbaudo@gmail.com
  November 2013
*/
class TupleMaker {
public:
    TupleMaker(const std::string &outFilename, const std::string &treename);
    ~TupleMaker();
    bool fill(const Susy::Lepton &l0, const Susy::Lepton &l1,
              const LeptonVector &otherLeptons, const JetVector &jets);
private: // rule of three 
    TupleMaker(const TupleMaker&);
    TupleMaker& operator=(const TupleMaker&);
private:
    bool init(const std::string &outFilename, const std::string &treename);
    bool initFile(const std::string &outFilename);
    bool initTree(const std::string &treename);
    bool initTreeBranches();
    bool close();
private:
    TFile *file_;
    TTree *tree_;
    FourMom l0_, l1_, met_;
    std::vector<FourMom> jets_, lowptLepts_;
    EventParameters eventPars_;
}; // end TupleMaker

} // namespace wh
} // namespace susy

#endif // end include guard
