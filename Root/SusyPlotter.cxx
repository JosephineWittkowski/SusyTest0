#include <iomanip>
#include "SusyNtuple/SusyDefs.h"
#include "SusyTest0/SusyPlotter.h"
//--DG-- #include "SusyMatrixMethod/DiLeptonMatrixMethod.h"

using namespace std;
using namespace Susy;


// 
// Histogram bins
//

const float varptbins[] = {0,10,20,30,40,50,70,100,150,200,250};
const int    varnptbins = 10;

const float ptmin    = 0;
const float ptmax    = 250;
const int  nptbins   = 25;

const float etamin   = 0;
const float etamax   = 5;
const int   netabins = 25; 

const float dphimin   = 0;
const float dphimax   = 3.2;
const int   ndphibins = 15; 

const float drmin   = 0;
const float drmax   = 5;
const int   ndrbins = 25; 

const float massmin  = 0;
const float massmax  = 300;
const int  nmassbins = 30;

const float massminfine  = 0;
const float massmaxfine  = 300;
const int  nmassbinsfine = 60;

const float massminfiner  = 0;
const float massmaxfiner  = 300;
const int  nmassbinsfiner = 100;

const float massminj  = 0;
const float massmaxj  = 1000;
const int  nmassbinsj = 100;

const int   njetbins = 5;
const float njetmin = -0.5;
const float njetmax = njetbins - 0.5;


/*--------------------------------------------------------------------------------*/
// Constructor
/*--------------------------------------------------------------------------------*/
SusyPlotter::SusyPlotter() : 
  SusySelection(),
  m_doLepSF(false),
  m_doTrigW(false),
  m_doFake(false),
  m_doCF(false)
{

  /*
  for(uint iPR=0; iPR<PR_N; ++iPR){
    for(uint iCh=0; iCh<Ch_N; ++iCh){
      
      #define NULLIFY(name) h_ ##name[iPR][iCh] = NULL

      NULLIFY(l0_pt);
      NULLIFY(l1_pt);
      NULLIFY(e_pt);
      NULLIFY(m_pt);
      NULLIFY(l0_eta);
      NULLIFY(l1_eta);
      NULLIFY(e_eta);
      NULLIFY(m_eta);
      NULLIFY(ll_M);
      NULLIFY(met);
      NULLIFY(metrel);

      #undef NULLIFY

    }
  }
  */
}

/*--------------------------------------------------------------------------------*/
// The Begin() function is called at the start of the query.
/*--------------------------------------------------------------------------------*/
void SusyPlotter::Begin(TTree* /*tree*/)
{
  SusySelection::Begin(0);
  if(m_dbg) cout << "SusyPlotter::Begin" << endl;
  
  // Do not dump the event counts
  m_dumpCounts = false;

  // systematics
  setSysts();

  // open the output file
  //system("mkdir -p anaplots");
  string append = "AnaHists";
  if(m_doFake) append = "FakeHists";
  if(m_doCF)   append = "ChargeFlipHists";
  if(m_histFileName.empty()) m_histFileName = "anaplots/"+m_sample+"." + append + ".root";
  m_histFile = new TFile(m_histFileName.c_str(), "recreate");
  TH1::SetDefaultSumw2(true);

  m_histFile->cd();
  //m_histFile->mkdir( sysNames[sys].c_str() ) -> cd();

  // Plot Region names
  for(uint iPR=0; iPR<PR_N; ++iPR){
    string PR = PRNames[iPR];

    // lepton channel loop
    for(uint iCh=0; iCh<Ch_N; ++iCh){
      string chan = chanNames[iCh];
      
      for(uint iSys=0; iSys<m_systs.size(); ++iSys){
	string sys = m_systNames.at(iSys);
	//cout<<"Sys: "<<sys<<endl;

	// Preprocessor convenience
	// make a histogram by name (leave off the "h_") and binning
        #define NEWHIST(name, xLbl, nbin, min, max)				\
	  do{								\
	    h_ ## name[iCh][iPR][iSys] = new TH1F((PR+"_"+chan+"_"+#name+"_"+sys).c_str(), #name ";" xLbl, nbin, min, max); \
	    h_ ## name[iCh][iPR][iSys]->Sumw2();				\
	  }while(0)
        #define NEWHIST2(name, xLbl, nbin, min, max)				\
	  do{								\
	    h_ ## name[iCh][iPR][iSys] = new TH2F((PR+"_"+chan+"_"+#name+"_"+sys).c_str(), #name ";" xLbl, nbin, min, max, nbin, min, max); \
	    h_ ## name[iCh][iPR][iSys]->Sumw2();				\
	  }while(0)
      
        #define NEWVARHIST(name, xLbl, nbin, bins)				\
	  do{								\
	    h_ ## name[iCh][iPR][iSys] = new TH1F((PR+"_"+chan+"_"+#name+"_"+sys).c_str(), #name ";" xLbl, nbin, bins); \
	    h_ ## name[iCh][iPR][iSys]->Sumw2();				\
	  }while(0)

         // Pt Plots
	NEWHIST(l0_pt, "l_{0} P_{T}", nptbins, ptmin, ptmax);
	NEWHIST(j0_pt, "j_{0} P_{T}", nptbins, ptmin, ptmax);
	NEWHIST(l1_pt, "l_{1} P_{T}", nptbins, ptmin, ptmax);
	NEWHIST(e_pt, "Electron P_{T}", nptbins, ptmin, ptmax);
	NEWHIST(m_pt, "Muon P_{T}", nptbins, ptmin, ptmax);
	
	// Eta Plots
	NEWHIST(l0_eta, "l_{0} #eta", netabins, etamin, etamax);
	NEWHIST(j0_eta, "j_{0} #eta", netabins, etamin, etamax);
	NEWHIST(l1_eta, "l_{1} #eta", netabins, etamin, etamax);
	NEWHIST(e_eta, "Electron #eta", netabins, etamin, etamax);
	NEWHIST(m_eta, "Muon #eta", netabins, etamin, etamax);
	
	// Mass plots
	NEWHIST(ll_M, "m(ll)", nmassbins, massmin, massmax);
	NEWHIST(ll_M_dPhiReg, "m(ll)", nmassbins, massmin, massmax);
	NEWHIST(ll_M_pos, "m(ll)", nmassbins, massmin, massmax);
	NEWHIST(ll_M_neg, "m(ll)", nmassbins, massmin, massmax);
	NEWHIST(llj_M, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(llj_M_pos, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(llj_M_neg, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(llj_M_mll_90_120, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(llj_M_mll_90_120_pos, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(llj_M_mll_90_120_neg, "m(llj)", nmassbins, massmin, massmax);
	NEWHIST(met_j_M, "m(met,j)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_M, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_j_ll_M, "m(met,j,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_j_Mt, "m(met,j)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_j_ll_Mt, "m(met,j,ll)", nmassbinsj, massminj, massmaxj);
	
	NEWHIST(met_l0_Mt, "m(met,l0)", nmassbins, massmin, massmax);
	NEWHIST(met_l1_Mt, "m(met,l0)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt_noj, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt_onej, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt_twoj, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt_ge3j, "m(met,ll)", nmassbins, massmin, massmax);
	NEWHIST(met_ll_Mt_oneOrtwoj, "m(met,ll)", nmassbins, massmin, massmax);

	// Met plots
	NEWHIST(met, "#slash{E}_{T}", nptbins, ptmin, ptmax);
	NEWHIST(metrel, "#slash{E}^{rel}_{T}", nptbins, ptmin, ptmax);
	
	// Met terms
	NEWHIST(met_refEle,   "#slash{E}_{T} (refEle)", nptbins, ptmin, ptmax);
	NEWHIST(met_refMuo,   "#slash{E}_{T} (refMuo)", nptbins, ptmin, ptmax);
	NEWHIST(met_refJet,   "#slash{E}_{T} (refJet)", nptbins, ptmin, ptmax);
	NEWHIST(met_softJet,  "#slash{E}_{T} (softJet)", nptbins, ptmin, ptmax);
	NEWHIST(met_refGamma, "#slash{E}_{T} (refGamma)", nptbins, ptmin, ptmax);
	NEWHIST(met_refCell,  "#slash{E}_{T} (refCell)", nptbins, ptmin, ptmax);


	// njet plots
	NEWHIST(njets, "# jets", njetbins, njetmin, njetmax);
	NEWHIST(njets_pos, "# jets leading pos", njetbins, njetmin, njetmax);
	NEWHIST(njets_neg, "# jets leading neg", njetbins, njetmin, njetmax);
	NEWHIST(njets_mll_90_120, "# jets", njetbins, njetmin, njetmax);	
	NEWHIST(njets_mll_90_120_pos, "# jets", njetbins, njetmin, njetmax);	
	NEWHIST(njets_mll_90_120_neg, "# jets", njetbins, njetmin, njetmax);	
	NEWHIST(nbjets, "# b jets", njetbins, njetmin, njetmax);
	NEWHIST(nbjets_mll_90_120, "# b jets", njetbins, njetmin, njetmax);
	NEWHIST(nbjets_mll_90_120_pos, "# b jets", njetbins, njetmin, njetmax);
	NEWHIST(nbjets_mll_90_120_neg, "# b jets", njetbins, njetmin, njetmax);

	NEWHIST(nfjets, "# f jets", njetbins, njetmin, njetmax);
	NEWHIST(nfjets_mll_90_120, "# f jets", njetbins, njetmin, njetmax);

	// Type and origin
	NEWHIST(l_type, "l_type", nType, Typemin, Typemax);
	NEWHIST(l_origin, "l_origin", nOrigin, Originmin, Originmax);

	// One bin for counting
	NEWHIST(onebin, "onebin", 1, -0.5, 0.5);

	// Sum charge
	NEWHIST(sumQ, "SumQ", 5, -2.5, 2.5);

	NEWHIST(dPhi_llmet_j, "dPhi(llmet,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dR_llmet_j, "dR(llmet,j)", ndrbins,drmin, drmax);

	NEWHIST(dPhi_met_l0, "dPhi(met,l0)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_met_l1, "dPhi(met,l1)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_met_ll, "dPhi(met,ll)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_met_j, "dPhi(met,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_ll_j, "dPhi(ll,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_l0_j, "dPhi(l0,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_l1_j, "dPhi(l1,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_l0_l1, "dPhi(l0,l1)", ndphibins, dphimin, dphimax);

	NEWHIST(dPhi_woSig_llmet_j, "dPhi(llmet,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_met_l0, "dPhi(met,l0)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_met_l1, "dPhi(met,l1)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_met_ll, "dPhi(met,ll)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_met_j, "dPhi(met,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_ll_j, "dPhi(ll,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_l0_j, "dPhi(l0,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_l1_j, "dPhi(l1,j)", ndphibins, dphimin, dphimax);
	NEWHIST(dPhi_woSig_l0_l1, "dPhi(l0,l1)", ndphibins, dphimin, dphimax);

	NEWHIST2(l0_l1_pt, "l0 vs l1 pt", nptbins, ptmin, ptmax);


	// Fine
	NEWHIST(ll_M_fine, "m(ll)", nmassbinsfine, massminfine, massmaxfine);
	NEWHIST(ll_M_finer, "m(ll)", nmassbinsfiner, massminfiner, massmaxfiner);
	// test histogram
	//NEWHIST(met_test, "test", nptbins, ptmin, ptmax);
	//NEWHIST(met_test2, "test2", nptbins, ptmin, ptmax);

        #undef NEWHIST
        #undef NEWHIST2
        #undef NEWVARHIST

      }// end loop over systematics
    }// end loop over channels
  }// end loop over Plot regions

}

/*--------------------------------------------------------------------------------*/
// Main process loop function 
/*--------------------------------------------------------------------------------*/
Bool_t SusyPlotter::Process(Long64_t entry)
{

  if(m_dbg) cout<<"SusyPlotter::Process"<<endl;
  // Communicate tree entry number to SusyNtObject
  GetEntry(entry);
  clearObjects();

  if(m_do1fb && !is1fb()){ return kTRUE; }

  // Chain entry not the same as tree entry
  static Long64_t chainEntry = -1;
  chainEntry++;
  if(m_dbg || chainEntry%50000==0)
  {
    cout << "**** Processing entry " << setw(6) << chainEntry
         << " run " << setw(6) << nt.evt()->run
         << " event " << setw(7) << nt.evt()->event << " ****" << endl;
  }

  // select signal objects
  selectObjects();

  // Tight Loose Check
  //if(m_baseLeptons.size() == 2 && passTrigger(m_baseLeptons)
  //&& (!nt.evt()->isMC || isTrueDilepton(m_baseLeptons))){ // basic

  ////VRTL
  //float weight = getEvtWeight(m_baseLeptons);
  //if( passVRTL(m_baseLeptons, m_signalJets2Lep, m_met) )
  //fillHistos(m_baseLeptons, m_signalJets2Lep, m_met, weight, PR_VRTL);

  //}

  // Check Analysis level cuts
  if( !selectAnaEvent(m_signalLeptons, m_baseLeptons) )    return kTRUE;
  if( nt.evt()->isMC && !isTrueDilepton(m_signalLeptons) ) return kTRUE;
  //if( (m_signalLeptons[0]->mcOrigin == 9 ||
  //m_signalLeptons[1]->mcOrigin == 9 ) ) return kTRUE;

  float weight = getEvtWeight(m_signalLeptons);

  
  fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_NONE);

  if(sameSign(m_signalLeptons))                           fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SSInc);
  else                                                    fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_OSInc);

  if( passSR1(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR1);
  if( passSR2(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR2);
  if( passSR3(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR3);
  if( passSR4(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR4);
  if( passSR4b(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR4b);
  //if( passSR5(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR5);
  //if( passZwindow(m_signalLeptons) )                  fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_Zwindow);
  if( passVR1(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_VR1);
  if( passVR2(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_VR2);
  if( passVR3(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_VR3);
  if( passVR4(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_VR4);
  if( passBR1(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_BR1);
  if( passBR2(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_BR2);
  if( passBR3(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_BR3);
  if( passBR4(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_BR4);

  if( passSR6(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR6);
  if( passSR7(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR7);
  if( passSR8(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR8);
  if( passSR9(m_signalLeptons, m_signalJets2Lep, m_met) ) fillHistos(m_signalLeptons, m_signalJets2Lep, m_met, weight, PR_SR9);

  return kTRUE;
}

/*--------------------------------------------------------------------------------*/
// The Terminate() function is the last function to be called during a query
/*--------------------------------------------------------------------------------*/
void SusyPlotter::Terminate()
{
  SusySelection::Terminate();
  if(m_dbg) cout << "SusyPlotter::Terminate" << endl;
  
  // Save the output
  m_histFile->Write();
  m_histFile->Close();
}

/*--------------------------------------------------------------------------------*/
// Fill histograms
/*--------------------------------------------------------------------------------*/
void SusyPlotter::fillHistos(const LeptonVector& leps, const JetVector &jets, const Met* met, 
			     const float weight, PlotRegion PR, uint sys)
{

  if(m_dbg) cout << "SusyPlotter::fillHistos" << endl;

  // Get Channel for leptons
  // ** Only dealing with exactly two leptons 
  if( leps.size() != 2 ) return;
  int ch = getChan(leps);


  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//
  // Some useful Definitions
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-//
  
  const Lepton* l0 = leps[0];
  const Lepton* l1 = leps[1];


  #define FILL(h, var)					\
    do{								\
      float max   = h[ch][PR][sys]->GetXaxis()->GetXmax();	\
      float xfill = var > max ? max - 1e-4 : var;		\
      h[ch][PR][sys]->Fill(xfill,weight);			\
      h[Ch_all][PR][sys]->Fill(xfill,weight);			\
    }while(0)

  #define FILL2(h, varx, vary)						\
    do{									\
      float maxx   = h[ch][PR][sys]->GetXaxis()->GetXmax();	\
      float xfill = varx > maxx ? maxx - 1e-4 : varx;		\
      float maxy   = h[ch][PR][sys]->GetYaxis()->GetXmax();	\
      float yfill = vary > maxy ? maxy - 1e-4 : vary;		\
      h[ch][PR][sys]->Fill(xfill,yfill,weight);			\
      h[Ch_all][PR][sys]->Fill(xfill,yfill,weight);			\
    }while(0)
  
  
  // Pt Plots
  FILL(h_l0_pt, l0->Pt());
  FILL(h_l1_pt, l1->Pt());
  if(l0->isEle()) FILL(h_e_pt, l0->Pt());
  else            FILL(h_m_pt, l0->Pt());
  if(l1->isEle()) FILL(h_e_pt, l1->Pt());
  else            FILL(h_m_pt, l1->Pt());

  // Eta plots
  FILL(h_l0_eta, l0->Eta());
  FILL(h_l1_eta, l1->Eta());
  if(l0->isEle()) FILL(h_e_eta, fabs(l0->Eta()));
  else            FILL(h_m_eta, fabs(l0->Eta()));
  if(l1->isEle()) FILL(h_e_eta, fabs(l1->Eta()));
  else            FILL(h_m_eta, fabs(l1->Eta()));

  // Mass Plots
  FILL(h_ll_M, (*l0 + *l1).M());

  // Met histograms
  float metrel = getMetRel(met, leps, jets);
  FILL(h_met, met->Et);
  FILL(h_metrel, metrel);
  
  // Met terms
  FILL(h_met_refEle,   met->refEle);
  FILL(h_met_refMuo,   met->refMuo);
  FILL(h_met_refJet,   met->refJet);
  FILL(h_met_softJet,  met->softJet);
  FILL(h_met_refGamma, met->refGamma);
  FILL(h_met_refCell,  met->refCell);

  int nJ = jets.size() > 4 ? 4 : jets.size();
  int nbJ = numberOfCBJets(jets);
  int nfJ = numberOfFJets(jets);

  FILL(h_njets, nJ);
  FILL(h_nbjets, nbJ);
  FILL(h_nfjets, nfJ);
  if(l0->q > 0)
    FILL(h_njets_pos,nJ);
  else
    FILL(h_njets_neg, nJ);

  FILL(h_l_type, l0->mcType);
  FILL(h_l_origin, l0->mcOrigin);
  FILL(h_l_type, l1->mcType);
  FILL(h_l_origin, l1->mcOrigin);

  FILL(h_onebin, 0.);

  FILL(h_sumQ, l0->q + l1->q);
  if(l0->q > 0)
    FILL(h_ll_M_pos, (*l0+*l1).M());
  else
    FILL(h_ll_M_neg, (*l0+*l1).M());
  float mll = (*l0+*l1).M();
  if(90 <= mll && mll <= 110){
    FILL(h_njets_mll_90_120, nJ);
    FILL(h_nbjets_mll_90_120, nbJ);
    FILL(h_nfjets_mll_90_120, nfJ);
    if(nJ==1) FILL(h_llj_M_mll_90_120, (*l0+*l1+*jets.at(0)).M());
    if(l0->q > 0){
      FILL(h_njets_mll_90_120_pos, nJ);
      FILL(h_nbjets_mll_90_120_pos, nbJ);
      if(nJ==1) FILL(h_llj_M_mll_90_120_pos, (*l0+*l1+*jets.at(0)).M());
    }
    else{
      FILL(h_njets_mll_90_120_neg, nJ);
      FILL(h_nbjets_mll_90_120_neg, nbJ);
      if(nJ==1) FILL(h_llj_M_mll_90_120_neg, (*l0+*l1+*jets.at(0)).M());
    }

  }
  
  const TLorentzVector mlv = met->lv();
  const TLorentzVector ll  = *l0 + *l1;
  float mt_met_ll = sqrt(2*ll.Pt()*mlv.Et()*(1-cos(ll.DeltaPhi(mlv))));

  FILL(h_met_ll_M, (*l0 + *l1 + mlv).M());
  FILL(h_met_ll_Mt, mt_met_ll);

  if( fabs(l0->DeltaPhi(met->lv())) < 3 ){
    FILL(h_ll_M_dPhiReg, ll.M());
    FILL(h_ll_M_fine, ll.M());
    FILL(h_ll_M_finer, ll.M());
  }

  FILL(h_met_l0_Mt, Mt(*l0,met->lv()));
  FILL(h_met_l1_Mt, Mt(*l1,met->lv()));

  if(nJ==0) FILL(h_met_ll_Mt_noj, mt_met_ll);
  if(nJ==1) FILL(h_met_ll_Mt_onej, mt_met_ll);
  if(nJ==2) FILL(h_met_ll_Mt_twoj, mt_met_ll);
  if(nJ>=3) FILL(h_met_ll_Mt_ge3j, mt_met_ll);
  if(nJ==1||nJ==4) FILL(h_met_ll_Mt_oneOrtwoj, mt_met_ll);

  if(nJ==1){
    float mllj = (*l0+*l1+*jets.at(0)).M();
    FILL(h_llj_M, mllj);
    if(l0->q > 0) FILL(h_llj_M_pos, mllj);
    else           FILL(h_llj_M_neg, mllj);

    const Jet* jet = jets.at(0);
    FILL(h_met_j_M, (*jet+mlv).M());
    FILL(h_met_j_ll_M, Mt(*l0+*l1+*jet,mlv));
    FILL(h_met_j_Mt, Mt(*jet, mlv));
    FILL(h_met_j_ll_Mt, Mt(*l0 + *l1 + *jet , mlv));
    FILL(h_j0_pt, jet->Pt());
    FILL(h_j0_eta, fabs(jet->Eta()));

    // Angle plots
    FILL(h_dPhi_llmet_j, fabs(jet->DeltaPhi( *l0 + *l1 + mlv)));
    FILL(h_dR_llmet_j, fabs(jet->DeltaR( *l0 + *l1 + mlv)));
    FILL(h_dPhi_met_l0, fabs(met->lv().DeltaPhi(*l0)));
    FILL(h_dPhi_met_l1, fabs(met->lv().DeltaPhi(*l1)));
    FILL(h_dPhi_met_ll, fabs(met->lv().DeltaPhi(*l0+*l1)));
    FILL(h_dPhi_met_j, fabs(met->lv().DeltaPhi(*jet)));
    FILL(h_dPhi_l0_j, fabs(l0->DeltaPhi(*jet)));
    FILL(h_dPhi_l1_j, fabs(l1->DeltaPhi(*jet)));
    FILL(h_dPhi_ll_j, fabs((*l1+*l0).DeltaPhi(*jet)));
    FILL(h_dPhi_l0_l1, fabs(l0->DeltaPhi(*l1)));

    float mll = (*l0 + *l1).M();
    if(90 < mll && mll < 120)
      FILL2(h_l0_l1_pt,l0->Pt(),l1->Pt());

    if(!(90 < mll && mll < 120)){
      FILL(h_dPhi_woSig_llmet_j, fabs(jet->DeltaPhi( *l0 + *l1 + mlv)));
      FILL(h_dPhi_woSig_met_l0, fabs(met->lv().DeltaPhi(*l0)));
      FILL(h_dPhi_woSig_met_l1, fabs(met->lv().DeltaPhi(*l1)));
      FILL(h_dPhi_woSig_met_ll, fabs(met->lv().DeltaPhi(*l0+*l1)));
      FILL(h_dPhi_woSig_met_j, fabs(met->lv().DeltaPhi(*jet)));
      FILL(h_dPhi_woSig_l0_j, fabs(l0->DeltaPhi(*jet)));
      FILL(h_dPhi_woSig_l1_j, fabs(l1->DeltaPhi(*jet)));
      FILL(h_dPhi_woSig_ll_j, fabs((*l1+*l0).DeltaPhi(*jet)));
      FILL(h_dPhi_woSig_l0_l1, fabs(l0->DeltaPhi(*l1)));
    }

    

  }
  //h_met_test[ch][PR]->Fill(met->Et,weight);
  //h_met_test2[ch][PR]->Fill(met->Et,weight);
  
  
  #undef FILL
  #undef FILL2
}

/*--------------------------------------------------------------------------------*/
// Get lepton channel
/*--------------------------------------------------------------------------------*/
int SusyPlotter::getChan(const LeptonVector& leps)
{

  uint ie = 0;
  uint im = 0;
  for(uint i=0; i<leps.size(); ++i){
    if( leps.at(i)->isEle() ) ie++;
    else if( leps.at(i)->isMu() ) im++;
  }

  if( ie == 2 && im == 0 ) return Ch_ee;
  if( ie == 1 && im == 1 ) return Ch_em;
  if( ie == 0 && im == 2 ) return Ch_mm;
  
  cout<<"Not ee/mm/em... Number Electrons: "<<ie<<" Number Muons: "<<im<<endl;
  return Ch_N; // not in range

}

/*--------------------------------------------------------------------------------*/
// Get systematics
/*--------------------------------------------------------------------------------*/
void SusyPlotter::setSysts()
{
  if(!m_doFake){
    m_systs.push_back(NtSys_NOM);  m_systNames.push_back(SusyNtSystNames[NtSys_NOM]);
  }
  else {
    cout<<"SusyPlotter::setSysts() : not implemented (DG Jan2013)"<<endl;
  }
//--DG--  else{
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_NONE);
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_NONE]);
//--DG--  }
//--DG--  /*
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_EL_RE_UP);   
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_EL_RE_UP]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_EL_RE_DOWN); 
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_EL_RE_DOWN]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_MU_RE_UP);   
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_MU_RE_UP]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_MU_RE_DOWN); 
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_MU_RE_DOWN]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_EL_FR_UP);   
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_EL_FR_UP]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_EL_FR_DOWN); 
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_EL_FR_DOWN]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_MU_FR_UP);   
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_MU_FR_UP]);
//--DG--    m_systs.push_back(SusyMatrixMethod::SYS_MU_FR_DOWN); 
//--DG--    m_systNames.push_back(SusyMatrixMethod::systematic_names[SusyMatrixMethod::SYS_MU_FR_DOWN]);
//--DG--  }
//--DG--  */

}


/*--------------------------------------------------------------------------------*/
// *** TEMPORARY ***
/*--------------------------------------------------------------------------------*/
bool SusyPlotter::passZwindow(const LeptonVector& leps)
{
  // Just require events to be in the Z window
  if( passZVeto(leps) ) return false;
  return true;

}