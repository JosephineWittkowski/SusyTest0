#ifndef MeasureFakeRate2_h
#define MeasureFakeRate2_h


//////////////////////////////////////////////////////////
// Code to measure the fake rates and real efficiencies //
// from a variety of control regions. Output will be    //
// root files that can then be used to calc. weights.   //
//////////////////////////////////////////////////////////

// Root Packages
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TProfile.h"

// Susy Packages
#include "SusyMatrixMethod/FakeRegions.h"
#include "SusyTest0/DileptonChannel.h"
#include "SusyTest0/FakeLeptonSources.h"
#include "SusyTest0/SusySelection.h"
#include "SusyTest0/EffObject.h"
#include "SusyTest0/SsPassFlags.h"

#include <vector>

using namespace std;
using namespace Susy;

enum WeightType
{
  WT_Raw = 0,   // weight = 1;
  WT_Evt,       // weight = gen weight
  WT_PU,        // weight = pileup weight
  WT_PU1fb,     // weight = pileup weight for 1/fb
  WT_LSF,       // weight = lepton SF
  WT_Btag,      // weight = btag
  WT_Trig,      // Trigger weight
  WT_AllAB3,    // all weights for A-B3
  WT_AllAE,     // all weights for A-E
  WT_N
};

class MeasureFakeRate2 : public SusySelection
{

 public:
  MeasureFakeRate2();
  virtual ~MeasureFakeRate2();
  virtual void    Begin(TTree *tree);
  virtual void    Terminate();
  virtual Bool_t  Process(Long64_t entry);
  void initHistos(string outName);
  MeasureFakeRate2& setFileName(string f){ m_fileName = f; return *this; }
  bool selectEvent(bool count=false);
  SsPassFlags passWhSS(const LeptonVector& leptons, const JetVector& jets, const Met* met);
  // Data Control Regions
  bool passRealCR(const LeptonVector &leptons, const JetVector& jets, const Met* met, susy::fake::Region CR);
  bool passHFCR(const LeptonVector &leptons, const JetVector& jets, const Met* met, susy::fake::Region CR);
  bool passConvCR(const LeptonVector &leptons, const JetVector& jets, const Met* met);
  bool passSignalRegion(const LeptonVector &leptons, const JetVector& jets, const Met* met, susy::fake::Region CR);
  // Monte Carlo Regions
  bool passMCReg(const LeptonVector &leptons, const JetVector& jets, const Met* met, susy::fake::Region CR);
  void fillRatesHistos(const Lepton* lep, const JetVector& jets, const Met* met, size_t regionIndex);
  // weight
  float getEvtWeight(const LeptonVector &leptons, bool includeBTag=false, bool includeTrig=true, bool doMediumpp=false);
  float getBTagWeight(const Event* evt);
  // counter methods
  void dumpEventCounters();
  void increment(float flag[], bool includeLepSF=false, bool includeBtag=false);
  void printCounter(string cut, float counter[ET_N][WT_N], int weight);
  void resetCounters();
  // Miscellaneous
  susy::fake::LeptonSource getLeptonSource(const Lepton* l);
  static const size_t kNmaxControlRegions=64, kNmaxLeptonTypes=2;
  const std::vector<susy::fake::Region> m_controlRegions; //!< where we compute SF and rates (pseudo t&p)
  const std::vector<susy::fake::Region> m_signalRegions;  //!< where we compute fractions to make the weighted avg
  const std::vector<susy::fake::Region> allRegions() const; //!< generate on the fly the sum of the two above
  enum LeptonType {kElectron, kMuon}; // DG Dec13: move this enum to a separate file when Fake is a separate package
  const std::vector<LeptonType> m_leptonTypes; //! types of leptons for which we will measure the fake probability
  static const std::string LeptonType2str(const LeptonType l);
  //! slightly different from criteria::isRealLepton; need to check whether this implementation is still needed (DG 2013-12-06)
  static bool isRealLepton(const Lepton* lep, uint dsid);
  static bool isFakeLepton(const Lepton* lep, uint dsid) { return !MeasureFakeRate2::isRealLepton(lep, dsid); }

  EffObject* h_l_pt         [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_l_pt_coarse  [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_l_eta        [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_l_eta_coarse [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_metrel       [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_met          [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_njets        [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_onebin       [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  EffObject* h_flavor       [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  TH1F*      h_l_pt_true    [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
  TH1F*      h_l_pt_fake    [kNmaxLeptonTypes][kNmaxControlRegions][susy::wh::Ch_N];
 protected:
  std::string  m_fileName;          // Outname file name
  TFile*       m_outFile;           // Output file
  LeptonVector m_probes;            // Probe lepton vector
  LeptonVector m_tags;              // Tag Lepton vector
  float        m_evtWeight;         // Event Weight
  float        m_metRel;            // Met Rel to be plotted
  int          m_ch;                // Set the channel
  DiLepEvtType m_ET;                // Dilepton event type to store cf

  // Event counters
  float                n_readin           [WT_N];
  float                n_pass_LAr         [WT_N];
  float                n_pass_BadJet      [WT_N];
  float                n_pass_BadMuon     [WT_N];
  float                n_pass_Cosmic      [WT_N];
  float                n_pass_atleast2Lep [WT_N];
  float                n_pass_exactly2Lep [WT_N];
  float                n_pass_mll20       [WT_N];
  float                n_pass_signalLep   [WT_N];
  float                n_pass_HFOR        [WT_N];
  float                n_pass_HotSpot     [WT_N];
  float                n_pass_TileError   [WT_N];
  float                n_pass_FEBCut      [WT_N];

  float                n_pass_flavor      [ET_N][WT_N];
  float                n_pass_mll         [ET_N][WT_N];    
  float                n_pass_signalTau   [ET_N][WT_N];
  float                n_pass_os          [ET_N][WT_N];
  float                n_pass_ss          [ET_N][WT_N];
  float                n_pass_evtTrig     [ET_N][WT_N];
  float                n_pass_trigMatch   [ET_N][WT_N];
  float                n_pass_truth       [ET_N][WT_N];

  float                n_pass_CRWHSS2lss  [ET_N][WT_N];
  float                n_pass_CRWHSStauv  [ET_N][WT_N];
  float                n_pass_CRWHSSmuiso [ET_N][WT_N];
  float                n_pass_CRWHSSeled0 [ET_N][WT_N];
  float                n_pass_CRWHSSnfj   [ET_N][WT_N];
  float                n_pass_CRWHSSnbj   [ET_N][WT_N];
  float                n_pass_CRWHSSnj    [ET_N][WT_N];
  float                n_pass_CRWHSS2lpt  [ET_N][WT_N];
  float                n_pass_CRWHSSzveto [ET_N][WT_N];
  float                n_pass_CRWHSSmwwt  [ET_N][WT_N];
  float                n_pass_CRWHSShtmin [ET_N][WT_N];
  float                n_pass_CRWHSSmetrel[ET_N][WT_N];
  float                n_pass_CRWHSS      [ET_N][WT_N];
};
#endif
