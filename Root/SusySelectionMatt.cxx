#include "SusyTest0/SusySelectionMatt.h"

#include <iomanip>
#include "TCanvas.h"

#include "SusyTest0/DileptonChannel.h"
#include "SusyTest0/criteria.h"

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
  m_doSusy(false),
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

  string per = "HCP";
  if(m_do1fb) per = "A-B3";
  if(m_doAD)  per = "A-D7";
  m_trigObj = new DilTrigLogic("Moriond",false/*No Reweight Utils!*/);
  if(m_useMCTrig) m_trigObj->useMCTrigger();

  if(m_doSusy){
    m_susyXS = new XSReader();
    m_susyXS->setDebug(m_dbg);
    m_susyXS->LoadXSInfo();
  }
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
  if( !passDeadRegions(m_preJets,
		       m_met,
		       nt.evt()->run,
		       nt.evt()->isMC)
      )                              return false;
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
  if( !passHfor() )                  return false;
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
bool SusySelectionMatt::selectAnaEvent(const LeptonVector& leptons, const LeptonVector& baseLeps,
                                       bool count)
{
  if(m_dbg) cout << "SusySelectionMatt::selectAnaEvent" << endl;
  if( !selectBaseEvent(true,count) )           return false;
  m_ET = getDiLepEvtType(baseLeps);
  if(m_ET == ET_me) m_ET = ET_em;
  // Check signal muon if we are counting; otherwise this is handled in the trigger package
  if( count && ( m_ET == ET_em || m_ET == ET_mm) )
    for(uint im=0; im<leptons.size(); ++im)
      if( leptons.at(im)->isMu() && fabs(leptons.at(im)->Eta()) > 2.4 ) return false;
  if( !passNLepCut(leptons) )    return false; // Signal Lepotn Cut
  if( !passTrigger(baseLeps) )   return false; // Trigger Requirement
  if( m_signalTaus.size() != 0 ) return false; // Reject if signal taus
  if(count) increment(n_pass_signalTau[m_ET]);
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
  if(sameSign(leptons)) { increment(n_pass_CRWHSS2lss  [m_ET], lsf, bsf); f.sameSign=true;} else return f;
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
bool SusySelectionMatt::passHfor()
{
  if(nt.evt()->hfor == 4 ) return false;
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passNLepCut(const LeptonVector& leptons)
{
  uint nLep = leptons.size();
  if(m_nLepMin>=0 && nLep < m_nLepMin) return false;
  if(m_nLepMax>=0 && nLep > m_nLepMax) return false;
  increment(n_pass_signalLep); //+=m_w;
  increment(n_pass_flavor[m_ET],true);
  // To stay inline with Anders' cutflow
  if(m_ET == ET_me) increment(n_pass_flavor[ET_em],true);
  if(m_ET == ET_em) increment(n_pass_flavor[ET_me],true);
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passNBaseLepCut(const LeptonVector& baseLeptons)
{
  if(m_cutNBaseLep){
    uint nLep = baseLeptons.size();
    if(m_nLepMin>=0 && nLep < m_nLepMin) return false;
    if(m_nLepMax>=0 && nLep > m_nLepMax) return false;
  }
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passTrigger(const LeptonVector& leptons)
{
  if(leptons.size() != 2)
    return false;
  bool passEvtTrig   = m_trigObj->passDilEvtTrig(leptons, m_met->Et, nt.evt());
  bool passTrigMatch = m_trigObj->passDilTrigMatch(leptons, m_met->Et, nt.evt());
  if( passEvtTrig ){
    increment(n_pass_evtTrig[m_ET],true);
  }
  if( passEvtTrig && passTrigMatch){
    increment(n_pass_trigMatch[m_ET],true);
    return true;
  }
  return false;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::sameFlavor(const LeptonVector& leptons)
{
  if( leptons.size() < 2 ) return false;
  return (leptons.at(0)->isMu() == leptons.at(1)->isMu());
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::oppositeFlavor(const LeptonVector& leptons)
{
  if( leptons.size() < 2 ) return false;
  return !(leptons.at(0)->isMu() == leptons.at(1)->isMu());
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::sameSign(const LeptonVector& leptons)
{
  if( leptons.size() < 2 ) return false;
  return leptons.at(0)->q * leptons.at(1)->q > 0;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::oppositeSign(const LeptonVector& leptons)
{
  return !(sameSign(leptons));
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passMll(const LeptonVector& leptons, float mll)
{
  if( leptons.size() < 2 ) return false;
  if( (*leptons.at(0) + *leptons.at(1)).M() < mll ) return false;
  increment(n_pass_mll[m_ET]);
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passBadMet(const Met* met, float cutval)
{
  JetVector preJets = getPreJets(&nt, NtSys_NOM);
  float RefEle = met->refEle;
  float px     = met->refJet_etx;
  float py     = met->refJet_ety;
  for(uint ij=0; ij<preJets.size(); ++ij){
    px += preJets.at(ij)->Px();
    py += preJets.at(ij)->Py();
  }
  float OJ_RefEle = sqrt(px*px+py*py)/ RefEle;
  return OJ_RefEle > cutval;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passJetVeto(const JetVector& jets)
{
  // Require no light, b, or forward jets
  int N_L25 = numberOfCLJets(jets);
  int N_B20 = numberOfCBJets(jets);
  int N_F30 = numberOfFJets(jets);
  return (N_L25 + N_B20 + N_F30 == 0);
}
/*--------------------------------------------------------------------------------*/
int SusySelectionMatt::nL20Close(const JetVector& jets)
{
  // number of L20 jets with dR > 1.0 to leading b jet
  Jet* leadingBjet = NULL;
  int nL20 = 0;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isCentralBJet(jets.at(ij)) ) continue;
    if( !leadingBjet ) leadingBjet = jets.at(ij);
    else if(leadingBjet->Pt() < jets.at(ij)->Pt())
      leadingBjet = jets.at(ij);
  }// end loop
  if( !leadingBjet ) return nL20;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isCentralLightJet(jets.at(ij)) ) continue;
    if( leadingBjet->DeltaR( *jets.at(ij) ) > 1.0 ) nL20++;
  }
  return nL20;
}
/*--------------------------------------------------------------------------------*/
int SusySelectionMatt::nB20Close(const JetVector& jets)
{
  // number of L20 jets with dR > 1.0 to leading b jet
  Jet* leadingBjet = NULL;
  int nB20 = 0;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isCentralBJet(jets.at(ij)) ) continue;
    if( !leadingBjet ) leadingBjet = jets.at(ij);
    else if(leadingBjet->Pt() < jets.at(ij)->Pt())
      leadingBjet = jets.at(ij);
  }// end loop
  if( !leadingBjet ) return nB20;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isCentralBJet(jets.at(ij)) ) continue;
    if( leadingBjet == jets.at(ij) )      continue;
    if( leadingBjet->DeltaR( *jets.at(ij) ) > 1.0 ) nB20++;
  }
  return nB20;
}
/*--------------------------------------------------------------------------------*/
int SusySelectionMatt::nF30Close(const JetVector& jets)
{
  // number of L20 jets with dR > 1.0 to leading b jet
  Jet* leadingBjet = NULL;
  int nF30 = 0;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isCentralBJet(jets.at(ij)) ) continue;
    if( !leadingBjet ) leadingBjet = jets.at(ij);
    else if(leadingBjet->Pt() < jets.at(ij)->Pt())
      leadingBjet = jets.at(ij);
  }// end loop
  if( !leadingBjet ) return nF30;
  for(uint ij=0; ij<jets.size(); ++ij){
    if( !isForwardJet(jets.at(ij)) ) continue;
    if( leadingBjet->DeltaR( *jets.at(ij) ) > 1.0 ) nF30++;
  }
  return nF30;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passbJetVeto(const JetVector& jets)
{
  // Reject if there is a b jet using 2L definition
  int N_B20 = numberOfCBJets(jets);
  return (N_B20 == 0);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passge2Jet(const JetVector& jets)
{
  int N_L25 = numberOfCLJets(jets);
  int N_B20 = numberOfCBJets(jets);
  int N_F30 = numberOfFJets(jets);
  return (N_L25 >=2 && N_B20 + N_F30 == 0);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passZVeto(const LeptonVector& leptons, float Zlow, float Zhigh)
{
  if( leptons.size() < 2 )   return false;
  if( !sameFlavor(leptons) ) return true;
  float mll = (*leptons.at(0) + *leptons.at(1)).M();
  if( Zlow < mll && mll < Zhigh ) return false;
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passMETRel(const Met *met, const LeptonVector& leptons,
				 const JetVector& jets, float metMax){
  if( getMetRel(met,leptons,jets) < metMax ) return false;
  return true;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passdPhi(TLorentzVector v0, TLorentzVector v1, float cut)
{
  return v0.DeltaPhi(v1) > cut;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::passMT2(const LeptonVector& leptons, const Met* met, float cut)
{
  float mt2 = getMt2(leptons, met);
  return (mt2 > cut);
}
/*--------------------------------------------------------------------------------*/
float SusySelectionMatt::getMt2(const LeptonVector& leptons, const Met* met)
{
  if( leptons.size() < 2 ) return false;
  TLorentzVector metlv = met->lv();
  TLorentzVector l0    = *leptons.at(0);
  TLorentzVector l1    = *leptons.at(1);
  double pTMiss[3] = {0.0, metlv.Px(), metlv.Py()};
  double pA[3]     = {0.0, l0.Px(), l0.Py()};
  double pB[3]     = {0.0, l1.Px(), l1.Py()};
  mt2_bisect::mt2 mt2_event;
  mt2_event.set_momenta(pA,pB,pTMiss);
  mt2_event.set_mn(0); // LSP mass = 0 is Generic
  return mt2_event.get_mt2();
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isRealLepton(const Lepton* lep)
{
  // Updated way of handling real and fake leptons using LeptonTruthTools
  // Need to handle new g2ww -- Assume all real for now
  uint mcId = nt.evt()->mcChannel;
  if( mcId == 169471 || mcId == 169472 || mcId == 169473 || mcId == 169474 ||
      mcId == 169475 || mcId == 169476 || mcId == 169477 || mcId == 169478 ||
      mcId == 169479)
    return true;
  return (lep->truthType == RecoTruthMatch::PROMPT);
  // Code taken from Steve.  There seems to be an issue with Sherpa samples, so
  // need to handle those separately. Also just for clarification:
  // * mcOrigin = 9 -- Tau Lepton
  // * mcType   = 1 -- Unknown Electron
  // * mcType   = 2 -- Iso Electron
  // * mcType   = 5 -- Unknown Muon
  // * mcType   = 6 -- Iso Muon
  // Cut is sample dependent due to Sherpa classifications broken
  // All tau leptons are classified as non-iso
  // I'm not sure why, yet, but for now I will treat them as real leptons.
  if(lep->mcOrigin == 9) return true;
  const int mcType = lep->mcType;
  // Sherpa diboson, assume all unknowns are real leptons
  // This is an approximation, but probably ok.
  if( (mcId>=126892 && mcId<=126895) || (mcId>=147770 && mcId<=147772) ||
      (mcId>=147774 && mcId<=147776)){
    if(lep->isEle()) return mcType == 1 || mcType == 2;
    else             return mcType == 5 || mcType == 6;
  }
  else{
    // 2-lep classifies everything as real if it
    // is from W, Z, tau, or top..
    //uint origin = lep->mcOrigin;
    //return origin == 9 || origin == 12 || origin == 13 || origin == 10;
    if(lep->isEle()) return mcType == 2;
    else             return mcType == 6;
  }
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isFakeLepton(const Lepton* lep)
{
  return !isRealLepton(lep);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isConvLepton(const Lepton* lep)
{
  //return lep->mcOrigin == 5;
  bool isConv       = lep->truthType == RecoTruthMatch::CONV;
  //bool isConv       = lep->mcOrigin == 5;
  bool isChargeFlip =  lep->isEle() ? ((Electron*) lep)->isChargeFlip : false;
  return isConv && !isChargeFlip;
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isHFLepton(const Lepton* lep)
{
  return (lep->truthType == RecoTruthMatch::HF);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isLFLepton(const Lepton* lep)
{
  return (lep->truthType == RecoTruthMatch::LF);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isQCDLepton(const Lepton* lep)
{
  return isHFLepton(lep) || isLFLepton(lep);
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isTrueDilepton(const LeptonVector &leptons)
{
  // Maybe not 100% kosher, but I just want to make sure I am not
  // double counting, so I require dilepton events to be real
  if( leptons.size() != 2 ) return false;
  bool l0_real = isRealLepton(leptons[0]);
  bool l1_real = isRealLepton(leptons[1]);
  bool l0_cf = leptons[0]->isEle() ? ((Electron*) leptons[0])->isChargeFlip : false;
  bool l1_cf = leptons[1]->isEle() ? ((Electron*) leptons[1])->isChargeFlip : false;
  return l0_real && l1_real && !l0_cf && !l1_cf; // ignoring charge flip
}
/*--------------------------------------------------------------------------------*/
bool SusySelectionMatt::isFakeDilepton(const LeptonVector &leptons)
{
  // Maybe not 100% kosher, but I just want to make sure I am not
  // double counting, so I require dilepton events to be real
  if( leptons.size() != 2 ) return false;
  bool l0_fake = isFakeLepton(leptons[0]);
  bool l1_fake = isFakeLepton(leptons[1]);
  bool l0_cf = leptons[0]->isEle() ? ((Electron*) leptons[0])->isChargeFlip : false;
  bool l1_cf = leptons[1]->isEle() ? ((Electron*) leptons[1])->isChargeFlip : false;
  return (l0_fake || l1_fake) && !l0_cf && !l1_cf; // ignoring charge flip
}
/*--------------------------------------------------------------------------------*/
float SusySelectionMatt::getEvtWeight(const LeptonVector& leptons, bool includeBTag, bool includeTrig,
				  bool doMediumpp)
{
  if( !nt.evt()->isMC ) return 1.;
  uint nl = leptons.size();
  float weight = 1;
  // lumi, xs, sumw, pileup
  if(m_do1fb) weight = getEventWeightAB3();
  else if(m_doAD)  weight = getEventWeightFixed(nt.evt()->mcChannel, LUMI_A_D);
  else if(m_doSusy){
    Event* evt = nt.evt();
    float weight = evt->w * evt->wPileup * LUMI_A_E / evt->sumw;
    weight *= m_susyXS->GetXS( evt->mcChannel );
  }
  else weight = getEventWeight(LUMI_A_L, true);
  // Trigger
  float trigW = 1;
  if(!m_useMCTrig && includeTrig){
    trigW  = nl == 2 ? m_trigObj->getTriggerWeight(leptons,
						   nt.evt()->isMC,
						   m_met->Et,
						   m_signalJets2Lep.size(),
						   nt.evt()->nVtx,
						   NtSys_NOM) : 1.;
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
    if( !(jet->Pt() > 20 && fabs(jet->detEta) < 2.4) ) continue;
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
/*--------------------------------------------------------------------------------*/
int SusySelectionMatt::getChan(const LeptonVector& leps)
{
  uint ie = 0;
  uint im = 0;
  for(uint i=0; i<leps.size(); ++i){
    if( leps.at(i)->isEle() ) ie++;
    else if( leps.at(i)->isMu() ) im++;
  }
  if( ie == 2 && im == 0 ) return susy::wh::Ch_ee;
  if( ie == 1 && im == 1 ) return susy::wh::Ch_em;
  if( ie == 0 && im == 2 ) return susy::wh::Ch_mm;
  cout<<"Not ee/mm/em... Number Electrons: "<<ie<<" Number Muons: "<<im<<endl;
  return susy::wh::Ch_N; // not in range
}
