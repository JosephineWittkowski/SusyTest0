#include "SusyTest0/SusySelectionMatt.h"

#include <iomanip>
#include "TCanvas.h"

#include "SusyTest0/criteria.h"
#include "SusyTest0/SusySelection.h" // passHfor

using namespace std;
using namespace Susy;

/*--------------------------------------------------------------------------------*/
// SusySelectionMatt Constructor
/*--------------------------------------------------------------------------------*/
SusySelectionMatt::SusySelectionMatt() :
  m_susyObj(NULL),
  m_trigObj(NULL),
  m_useMCTrig(false),
  m_w(1.0),
  m_do1fb(false),
  m_doAD(false),
  m_dumpCounts(true),
  m_nLepMin(2),
  m_nLepMax(2),
  m_cutNBaseLep(true),
  m_ET(ET_Unknown),
  m_susyXS(NULL)
{
  // Loop over weight types
  for(int w=0; w<WT_N; ++w){
    n_readin[w]       = 0;
    n_pass_LAr[w]     = 0;
    n_pass_BadJet[w]  = 0;
    n_pass_BadMuon[w] = 0;
    n_pass_Cosmic[w]  = 0;
    n_pass_atleast2Lep[w] = 0;
    n_pass_exactly2Lep[w] = 0;
    n_pass_mll20[w]       = 0;
    n_pass_signalLep[w]   = 0;
    n_pass_HFOR[w]        = 0;
    n_pass_HotSpot[w]     = 0;
    n_pass_TileError[w]   = 0;
    n_pass_FEBCut[w]      = 0;

    // The rest are channel specific.
    for(int i=0; i<ET_N; ++i){
      n_pass_flavor[i][w]   = 0;
      n_pass_signalTau[i][w]   = 0;
      n_pass_evtTrig[i][w]     = 0;
      n_pass_trigMatch[i][w]   = 0;
      n_pass_mll[i][w]         = 0;
      n_pass_ss[i][w]          = 0;
      n_pass_os[i][w]          = 0;
      n_pass_truth[i][w]       = 0;
      //
      n_pass_CRWHSS2lss  [i][w] = 0;
      n_pass_CRWHSStauv  [i][w] = 0;
      n_pass_CRWHSSmuiso [i][w] = 0;
      n_pass_CRWHSSeled0 [i][w] = 0;
      n_pass_CRWHSSnfj   [i][w] = 0;
      n_pass_CRWHSSnbj   [i][w] = 0;
      n_pass_CRWHSSnj    [i][w] = 0;
      n_pass_CRWHSS2lpt  [i][w] = 0;
      n_pass_CRWHSSzveto [i][w] = 0;
      n_pass_CRWHSSmwwt  [i][w] = 0;
      n_pass_CRWHSShtmin [i][w] = 0;
      n_pass_CRWHSSmetrel[i][w] = 0;
      n_pass_CRWHSS      [i][w] = 0;
    }
  }// end loop over weight types

  m_BoundLow = TF1("expo","expo",1.5,4.0);
  m_BoundLow.FixParameter(0, 6.11861);
  m_BoundLow.FixParameter(1,-1.118815);
}

/*--------------------------------------------------------------------------------*/
// The Begin() function is called at the start of the query.
/*--------------------------------------------------------------------------------*/
void SusySelectionMatt::Begin(TTree* /*tree*/)
{
  SusyNtAna::Begin(0);
  if(m_dbg) cout << "SusySelectionMatt::Begin" << endl;

  // Specify 2-lep ana type
  setAnaType(Ana_2LepWH);
  setSelectTaus(true);
  string period("Moriond");
  m_trigObj = new DilTrigLogic(period, false/*No Reweight Utils!*/);
  if(m_useMCTrig) m_trigObj->useMCTrigger();
}

/*--------------------------------------------------------------------------------*/
Bool_t SusySelectionMatt::Process(Long64_t entry)
{
  return kTRUE;
}
/*--------------------------------------------------------------------------------*/
void SusySelectionMatt::Terminate()
{
  SusyNtAna::Terminate();
  if(m_dbg) cout << "SusySelectionMatt::Terminate" << endl;
  if(m_dumpCounts)
    dumpEventCounters();
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::selectEvent(bool count)
{
  if(m_dbg) cout << "SusySelectionMatt::selectEvent" << endl;
  // Basic event cuts
  // Upstream Cuts:
  // * GRL
  // * LAr error
  // * tile Error
  // * TTC veto
  // * Primary Veretex
  int flag = nt.evt()->cutFlags[NtSys_NOM];
  // LAr flag (always true..)
  if( !passLAr(flag) )               return false;
  if(count) increment(n_pass_LAr);
  if(m_dbg) cout<<"\tPass LAr"<<endl;
  // Bad Jets
  if( hasBadJet(m_baseJets) )         return false;
  if(count) increment(n_pass_BadJet);
  if(m_dbg) cout<<"\tPass Bad jet"<<endl;
  // FEB issue -- maybe this is calo jets?
  if(!passDeadRegions(m_preJets, m_met, nt.evt()->run, nt.evt()->isMC)) return false;
  if(count) increment(n_pass_FEBCut);
  if(m_dbg) cout<<"\tPass Dead Regions"<<endl;
  // Bad Muons
  if( hasBadMuon(m_preMuons) )         return false;
  if(count) increment(n_pass_BadMuon);
  if(m_dbg) cout<<"\tPass Bad Muon"<<endl;
  // Cosmic Muons
  if( hasCosmicMuon(m_baseMuons) ) return false;
  if(count) increment(n_pass_Cosmic);
  if(m_dbg) cout<<"\tPass Cosmic"<<endl;
  // HFor
  if( !SusySelection::passHfor(nt) ) return false;
  if(count) increment(n_pass_HFOR);
  if(m_dbg) cout<<"\tPass Hfor"<<endl;
  // HotSpot
  if( hasHotSpotJet(m_preJets) )       return false;
  if(count) increment(n_pass_HotSpot);
  if(m_dbg) cout<<"\tPass Hot Spot"<<endl;
  // Tile Error
  if( !passTileTripCut(flag) )          return false;
  if(count) increment(n_pass_TileError);
  if(m_dbg) cout<<"\tPass Tile Error"<<endl;
  // If we are not counting (ie doing cutflow) then remove all the baseline muons with eta < 2.5
  if( !count ){
    LeptonVector temp = m_baseLeptons;
    m_baseLeptons.clear();
    for(uint il=0; il<temp.size(); ++il){
      Lepton* lep = temp.at(il);
      if( lep->isMu() && fabs(lep->Eta()) > 2.4) continue;
      m_baseLeptons.push_back(lep);
    }
  }
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::selectBaseEvent(bool doMll, bool count)
{
  // Make sure event level cuts are checked
  if( !selectEvent(count) )               return false;
  // Lepton Analysis cuts
  if( m_baseLeptons.size() < 2 )                return false;
  if(count) increment(n_pass_atleast2Lep);
  if( m_dbg ) cout<<"\tPassed at least 2 leptons"<<endl;
  if(m_baseLeptons.size() != 2)                 return false;
  if(count) increment(n_pass_exactly2Lep);
  if( m_dbg ) cout<<"\tPassed Exctly 2 leptons"<<endl;
  if(doMll && m_baseLeptons.size() == 2){
    if( Mll(m_baseLeptons[0], m_baseLeptons[1]) < 20 )
      return false;
    if(count) increment(n_pass_mll20);
    if( m_dbg ) cout<<"\tPass mll > 20"<<endl;
  }
  return true;
}
/*--------------------------------------------------------------------------------*/
SsPassFlags SusySelectionMatt::passWhSS(const LeptonVector& leptons, const JetVector& jets, const Met* met)
{
  // for now keep it simple:
  // - 2lep ss (I assume we don't need qFlip here)
  // - pt0>30
  // -  pt1>20 for em, ee
  // - ht>200
  // - d0 < 3 for el
  // - z veto (10GeV) for ee
  // - mww > 150 (ee), > 140 (em), >100 (mm)
  // - metrel > 50 for ee, em
  //
  // DG: be careful, this is slightly different from SusySelection::passSrSs.
  // In particular, here we don't require the trigger match and some
  // of the criteria are in a different order (e.g. same-sign).
  // At some point try to unify SusySelection with SusySelectionMatt.
  SsPassFlags f;
  bool lsf(false), bsf(false); // compute trigw and btagw only when accepting the event
  if(susy::sameSign(leptons)) { increment(n_pass_CRWHSS2lss  [m_ET], lsf, bsf); f.sameSign=true;} else return f;
  DiLepEvtType ll = m_ET = getDiLepEvtType(leptons);
  bool isee(ll==ET_ee), isem(ll==ET_em||ll==ET_me), ismm(ll==ET_mm);
  float ptL0Min  = 30;
  float ptL1Min  = (ismm ? 0.0 : 20.0);
  float htMin    = 200;
  bool applyMllZveto(isee);
  float mZ0(91.2);
  float loMllZ(applyMllZveto ? mZ0-10. : FLT_MAX);
  float hiMllZ(applyMllZveto ? mZ0+10. : FLT_MIN);
  float mtwwMin = (isee ? 150 : (isem ? 140 : (ismm ? 100 : FLT_MIN))); // todo : for now keep it simple, just one cut
  float metRelMin = (isee ? 50 : (isem ? 50 : (ismm ? FLT_MIN : FLT_MIN))); // for now simple

  if(m_signalTaus.size()==0)                          { increment(n_pass_CRWHSStauv  [m_ET], lsf, bsf); f.tauVeto=true;} else  return f;
  if(numberOfFJets(jets)==0)                          { increment(n_pass_CRWHSSnfj   [m_ET], lsf, bsf); f.fjveto =true;} else  return f;
  if(numberOfCBJets(jets)==0)                         { increment(n_pass_CRWHSSnbj   [m_ET], lsf, bsf); f.bjveto =true;} else  return f;
  if(numberOfCLJets(jets)>0)                          { increment(n_pass_CRWHSSnj    [m_ET], lsf, bsf); f.ge1j   =true;} else  return f;
  if(susy::pass2LepPt    (leptons, ptL0Min, ptL1Min)) { increment(n_pass_CRWHSS2lpt  [m_ET], lsf, bsf); f.lepPt  =true;} else  return f;
  if(susy::passZllVeto   (leptons, loMllZ, hiMllZ))   { increment(n_pass_CRWHSSzveto [m_ET], lsf, bsf); f.zllVeto=true;} else  return f;
  if(susy::passMtLlMetMin(leptons, met, mtwwMin))     { increment(n_pass_CRWHSSmwwt  [m_ET], lsf, bsf); f.mtllmet=true;} else  return f;
  if(susy::passHtMin     (leptons, jets, met, htMin)) { increment(n_pass_CRWHSShtmin [m_ET], lsf, bsf); f.ht     =true;} else  return f;
  if(getMetRel(met,leptons,jets)>metRelMin)           { increment(n_pass_CRWHSSmetrel[m_ET], lsf, bsf); f.metrel =true;} else  return f;
  lsf = bsf = true;
  increment(n_pass_CRWHSS[m_ET], lsf, bsf);
  return f;
}
/*--------------------------------------------------------------------------------*/
// Generic cuts
/*--------------------------------------------------------------------------------*/
float SusySelectionMatt::getEvtWeight(const LeptonVector& leptons, bool includeBTag, bool includeTrig,
				  bool doMediumpp)
{
  if( !nt.evt()->isMC ) return 1.;
  uint nl = leptons.size();
  float weight = 1;
  weight = getEventWeight(LUMI_A_L, true); // lumi, xs, sumw, pileup
  // Trigger
  float trigW = 1;
  if(!m_useMCTrig && includeTrig){
      trigW  = (nl == 2 ?
                m_trigObj->getTriggerWeight(leptons,
                                            nt.evt()->isMC,
                                            m_met->Et,
                                            m_signalJets2Lep.size(),
                                            nt.evt()->nVtx,
                                            NtSys_NOM)
                : 1.0);
    if(trigW != trigW){ cout<<"\tTrigger weight: "<<trigW<<endl; trigW =0; }// deal with NaN
    if(trigW < 0) trigW = 0;
  }
  float effW   = nl > 0 ? leptons[0]->effSF : 1.;
  effW        *=  nl > 1 ? leptons[1]->effSF : 1.;
  // btag, if included
  float bTag   =  includeBTag ? getBTagWeight(nt.evt()) : 1.;
  return weight * trigW * effW * bTag;
}
/*--------------------------------------------------------------------------------*/
float SusySelectionMatt::getBTagWeight(const Event* evt)
{
  JetVector tempJets;
  for(uint ij=0; ij<m_baseJets.size(); ++ij){
    Jet* jet = m_baseJets.at(ij);
    if( !(jet->Pt() > 20 && fabs(jet->detEta) < JET_ETA_CUT_2L) ) continue;
    tempJets.push_back(jet);
  }
  return bTagSF(evt, tempJets, evt->mcChannel, BTag_NOM);
}
/*--------------------------------------------------------------------------------*/
void SusySelectionMatt::dumpEventCounters()
{
  string v_WT[] = {"Raw","Event","Pileup","Pileup A-B3",
                   "LeptonSF","btagSF","TrigSF","All A-B3", "All A-E"};

  for(int w=0; w<WT_N; ++w){
    cout << "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" << endl;
    cout << "SusySelectionMatt Event counts for weight: " << v_WT[w] << endl;
    cout << endl;
    cout << "read in:       " << n_readin[w]           << endl;
    cout << "pass LAr:      " << n_pass_LAr[w]         << endl;
    cout << "pass BadJet:   " << n_pass_BadJet[w]      << endl;
    cout << "pass FEB:      " << n_pass_FEBCut[w]      << endl;
    cout << "pass BadMu:    " << n_pass_BadMuon[w]     << endl;
    cout << "pass Cosmic:   " << n_pass_Cosmic[w]      << endl;
    cout << "pass HFOR:     " << n_pass_HFOR[w]        << endl;
    cout << "pass Hot Spot: " << n_pass_HotSpot[w]     << endl;
    cout << "pass Tile Err: " << n_pass_TileError[w]   << endl;
    cout << "   ------  Start Comparison Here ------ " << endl;
    cout << ">=2 Lep        " << n_pass_atleast2Lep[w] << endl;
    cout << "pass Exactly 2 " << n_pass_exactly2Lep[w] << endl;
    cout << "pass mll > 20  " << n_pass_mll20[w]       << endl;
    cout << "pass nSigLep:  " << n_pass_signalLep[w]   << endl;

    cout << "************************************" << endl;

     cout << "Cut                   \tee\t\tmm\t\tem" << endl;
    printCounter("pass flavor:     " , n_pass_flavor    , w);
    printCounter("pass evt trig:   " , n_pass_evtTrig   , w);
    printCounter("pass trig match: " , n_pass_trigMatch , w);
    printCounter("pass tau veto:   " , n_pass_signalTau , w);
    printCounter("pass Truth:      " , n_pass_truth     , w);
    printCounter("pass OS:         " , n_pass_os        , w);
    printCounter("pass SS:         " , n_pass_ss        , w);
    cout << "-----------------------------------------------------"   << endl;
    cout << "Cut                   \tee\t\tmm\t\tem" << endl;
    printCounter("pass: WHSS 2lss    ", n_pass_CRWHSS2lss  , w);
    printCounter("pass: WHSS tauveto ", n_pass_CRWHSStauv  , w);
    printCounter("pass: WHSS muiso   ", n_pass_CRWHSSmuiso , w);
    printCounter("pass: WHSS eld0    ", n_pass_CRWHSSeled0 , w);
    printCounter("pass: WHSS fjveto  ", n_pass_CRWHSSnfj   , w);
    printCounter("pass: WHSS bjveto  ", n_pass_CRWHSSnbj   , w);
    printCounter("pass: WHSS nj      ", n_pass_CRWHSSnj    , w);
    printCounter("pass: WHSS 2lpt    ", n_pass_CRWHSS2lpt  , w);
    printCounter("pass: WHSS zveto   ", n_pass_CRWHSSzveto , w);
    printCounter("pass: WHSS mwwt    ", n_pass_CRWHSSmwwt  , w);
    printCounter("pass: WHSS htmin   ", n_pass_CRWHSShtmin , w);
    printCounter("pass: WHSS metrel  ", n_pass_CRWHSSmetrel, w);
    printCounter("pass: WHSS         ", n_pass_CRWHSS      , w);
  }// end loop over weight type

}

/*--------------------------------------------------------------------------------*/
void SusySelectionMatt::printCounter(string cut, float counter[ET_N][WT_N], int weight)
{
  cout << cut;
  for(int i=0; i<ET_N-2; ++i)
    cout << "\t" << Form("%10.3f",counter[i][weight]);
  cout << endl;
}
