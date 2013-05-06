#ifndef SusyAna_SusyPlotter_h
#define SusyAna_SusyPlotter_h

#include "SusyTest0/SusySelection.h"
#include "SusyTest0/SusyAnaDefs.h"

#include "TH1F.h"
#include "TH2F.h"


/*

    SusyPlotter - class for making analysis histograms

*/

// lepton channel
enum Chan {
  Ch_all = 0,
  Ch_ee,
  Ch_mm,
  Ch_em,
  Ch_N
};

// Lepton chan names
static string chanNames[] = {
  "all",
  "ee",
  "mm",
  "em"
};

// Plotting regions
enum PlotRegion{
  PR_NONE,
  PR_SR6base,
  PR_SR6,
  PR_SR7base,
  PR_SR7Nj,
  PR_SR7NjZttVeto,
  PR_SR7NjPtTot,
  PR_SR7NjMll,
  PR_SR7,
  PR_SR8base,
  PR_SR8,
  PR_SR9base,
  PR_SR9,
  PR_N
};

static string PRNames[] = {
  "srnone",
  "sr6base",
  "sr6",
  "sr7base",
  "sr7Nj",
  "sr7NjZttVeto",
  "sr7NjPtTot",
  "sr7NjMll",
  "sr7",
  "sr8base",
  "sr8",
  "sr9base",
  "sr9"
};

class SusyPlotter : public SusySelection
{

  public:

    SusyPlotter();
    virtual ~SusyPlotter(){};



    // Begin is called before looping on entries
    virtual void    Begin(TTree *tree);
    // Terminate is called after looping is finished
    virtual void    Terminate();

    // Main event loop function
    virtual Bool_t  Process(Long64_t entry);

    //
    // Method to fill histograms
    //
    void fillHistos(const LeptonVector& leps, const JetVector &jets, const Met* met,
		    const float weight, PlotRegion PR = PR_NONE, uint sys = 0);

    //
    // Define some additional plot regions
    //

    bool passZwindow(const LeptonVector& leps);

    //
    // Miscellaneous methods
    //

    // compute Mt
    float Mt(TLorentzVector p1, TLorentzVector met){
      return sqrt(2*p1.Pt()*met.Et()*(1-cos(p1.DeltaPhi(met))));
    };

    // compute lepton channel
    int getChan(const LeptonVector& leps);

    // get list of systematics to consider
    // override in SusyMatrixMethod
    void setSysts();
 public:
    static float transverseMass(const TLorentzVector &lep, const TLorentzVector &met);
    //! compute tau-tau mass assuming that v's are collinear with leptons and responsible for all MET
    static float mZTauTau(const TLorentzVector &l0, const TLorentzVector &l1, const TLorentzVector &met);
    //! \f$ \Sum cos \Delta\phi \f$ used in CERN-PH-EP-2011-097
    static float sumCosDeltaPhi(const TLorentzVector &l0, const TLorentzVector &l1, const TLorentzVector &met);
    //! \f$ \Sum E_{T} + E_{T}^{miss} \f$ used in CERN-PH-EP-2011-097
    static float sumEtEtMiss(const TLorentzVector &el, const TLorentzVector &mu,
			     const JetVector &jets, const TLorentzVector &met);
    //! compute the number of kinematically allowed neutrino solutions
    /*! This code is based on hep-ph/0603011. The number of possible
      solutions ranges from 0 to 4. However, you should consider
      calling the function twice (swapping the two jets), unless you
      can tell which one is the b and which one is the bbar.
     */
    static int numberOfNeutrinoSolutions(const TLorentzVector &lPos, const TLorentzVector &lNeg,
					 const Jet &jet0, const Jet &jet1,
					 const TLorentzVector &met);


    ClassDef(SusyPlotter, 1);

  protected:

    std::vector<uint> m_systs;                // systematics to process
    std::vector<string> m_systNames;          // systematics to process

    std::string         m_histFileName;         // output histo file name
    TFile*              m_histFile;             // output histo file

    // control flags
    bool                m_doLepSF;              // use lepton efficiency SF
    bool                m_doTrigW;              // do trigger reweighting
    //bool                m_do1fb;                // use 1/fb weights
    bool                m_doFake;               // do Fake estimate
    bool                m_doCF;                 // do charge flip

    // preprocessor convenience - add more indices later
    #define DEFHIST( name ) h_ ## name[Ch_N][PR_N][40/*Guess for # of sys*/];

    // Pt
    TH1F* DEFHIST(l0_pt);
    TH1F* DEFHIST(l1_pt);
    TH1F* DEFHIST(e_pt);
    TH1F* DEFHIST(m_pt);
    TH1F* DEFHIST(ll_pt);
    TH1F* DEFHIST(tot_pt);

    TH1F* DEFHIST(j0_pt);
    TH1F* DEFHIST(j1_pt);

    // eta
    TH1F* DEFHIST(l0_eta);
    TH1F* DEFHIST(l1_eta);
    TH1F* DEFHIST(e_eta);
    TH1F* DEFHIST(m_eta);
    TH1F* DEFHIST(j0_eta);
    TH1F* DEFHIST(j1_eta);
    TH1F* DEFHIST(jj_deta);
    TH1F* DEFHIST(jj_drap);

    // Mass
    TH1F* DEFHIST(ll_M);
    TH1F* DEFHIST(jj_M);
    TH1F* DEFHIST(mtautau_l0l1met);

    // MET
    TH1F* DEFHIST(met);
    TH1F* DEFHIST(metrel);

    // # of jets
    TH1F* DEFHIST(njets);
    TH1F* DEFHIST(nbasejets);
    TH1F* DEFHIST(nbjets);

    // Type and origin
    TH1F* DEFHIST(l_type);
    TH1F* DEFHIST(l_origin);

    // One bin
    TH1F* DEFHIST(onebin);

    // Charge plots
    TH1F* DEFHIST(sumQ);
    TH1F* DEFHIST(ll_M_pos);
    TH1F* DEFHIST(ll_M_neg);
    TH1F* DEFHIST(njets_pos);
    TH1F* DEFHIST(njets_neg);
    TH1F* DEFHIST(nfjets);

    TH1F* DEFHIST(llj_M);
    TH1F* DEFHIST(llj_M_pos);
    TH1F* DEFHIST(llj_M_neg);

    // Met mass plots
    TH1F* DEFHIST(met_j_M);
    TH1F* DEFHIST(met_ll_M);
    TH1F* DEFHIST(met_j_ll_M);
    TH1F* DEFHIST(met_j_Mt);
    TH1F* DEFHIST(met_ll_Mt);
    TH1F* DEFHIST(met_j_ll_Mt);
    TH1F* DEFHIST(met_ll_Mt2);

    TH1F* DEFHIST(mt_ll_met);
    TH1F* DEFHIST(mt_l0_met);
    TH1F* DEFHIST(mt_l1_met);
    TH1F* DEFHIST(mt_l_met_min);
    TH1F* DEFHIST(mct_top_tag);
    TH1F* DEFHIST(sumJ0J1_mv1tag);
    TH1F* DEFHIST(numNeutrinoSol);

    TH1F* DEFHIST(dPhi_llmet_j);
    TH1F* DEFHIST(dPhi_met_l0);
    TH1F* DEFHIST(dPhi_met_l1);
    TH1F* DEFHIST(dPhi_met_ll);
    TH1F* DEFHIST(dPhi_met_j);
    TH1F* DEFHIST(dPhi_ll_j);
    TH1F* DEFHIST(dPhi_l0_j);
    TH1F* DEFHIST(dPhi_l1_j);
    TH1F* DEFHIST(dPhi_l0_l1);

    TH1F* DEFHIST(dPhi_ll_jj);
    TH1F* DEFHIST(dPhi_l0_jj);
    TH1F* DEFHIST(dPhi_l1_jj);
    TH1F* DEFHIST(dR_l0_l1);
    TH1F* DEFHIST(dR_ll_jj);


    TH1F* DEFHIST(dPhi_woSig_llmet_j);
    TH1F* DEFHIST(dPhi_woSig_met_l0);
    TH1F* DEFHIST(dPhi_woSig_met_l1);
    TH1F* DEFHIST(dPhi_woSig_met_ll);
    TH1F* DEFHIST(dPhi_woSig_met_j);
    TH1F* DEFHIST(dPhi_woSig_ll_j);
    TH1F* DEFHIST(dPhi_woSig_l0_j);
    TH1F* DEFHIST(dPhi_woSig_l1_j);
    TH1F* DEFHIST(dPhi_woSig_l0_l1);

    TH1F* DEFHIST(dR_llmet_j);

    TH2F* DEFHIST(l0_l1_pt);

    TH1F* DEFHIST(l0_qeta);
    TH1F* DEFHIST(l1_qeta);

    #undef DEFHIST

};

#endif
