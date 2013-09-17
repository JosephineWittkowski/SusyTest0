
#include <algorithm> // std::find
#include <cstdlib>
#include <iomanip> // std::setw
#include <iterator>  // std::distance
#include <string>
#include <vector>

#include "Cintex/Cintex.h"

#include "SusyTest0/FakePlotting.h"
#include "SusyTest0/myHist.h"
#include "SusyTest0/utils.h"

using namespace std;

/*

Plotting Macro

*/

void help(const vector<string> &opts)
{
  size_t i=0;
  cout << "  Options:"                            << endl;

  cout << "  -n number of events to process"      << endl;
  cout << "     defaults: -1 (all events)"        << endl;

  cout << "  -r run option number"                << endl;
  size_t width(24);
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Data (default)"                 << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : MC"                             << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Data and MC"                    << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Data - MC Rate"                 << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Photon + jet"                   << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Data/MC scale factors"          << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : MC Signal Regions rates"        << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : MC Composition in SR"           << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : TT/TL/LT/LL Matrix Pred Plots"  << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : dump Matrix Pred in SR"         << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : HF normalization"               << endl; i++;
  cout<<"  "<<setw(3)<<i<<" "<<setw(width)<<opts[i]<<" : Dump Percentages"               << endl; i++;

  cout << "  -h print this help"                << endl;
}


int main(int argc, char** argv)
{
  ROOT::Cintex::Cintex::Enable();

  int dbg = 0;
  RunOption ro = RO_Data;

  vector<string> txtOptions; // ugly duplication, but allows for mnemonic...fixme
  txtOptions.push_back("data"            );
  txtOptions.push_back("mc"              );
  txtOptions.push_back("data-and-mc"     );
  txtOptions.push_back("data-mc-rate"    );
  txtOptions.push_back("photon-jet"      );
  txtOptions.push_back("datamc-sf"       );
  txtOptions.push_back("mc-sr-rates"     );
  txtOptions.push_back("mc-sr-comp"      );
  txtOptions.push_back("matrix-pred-plot");
  txtOptions.push_back("matrix-pred-dump");
  txtOptions.push_back("hf-norm"         );
  txtOptions.push_back("percent-dump"    );

  cout << "FakePlot" << endl;
  cout << endl;

  /** Read inputs to program */
  for(int i = 1; i < argc; i++) {
    if      (strcmp(argv[i], "-d") == 0) { dbg = atoi(argv[++i]); }
    else if (strcmp(argv[i], "-r") == 0) {
      string sw(argv[++i]);
      if(isInt(sw)) ro = (RunOption) atoi(sw.c_str());
      else {
        vector<string>::iterator it = std::find(txtOptions.begin(), txtOptions.end(), sw);
        bool isValidOpt(it != txtOptions.end());
        if(isValidOpt) { ro = (RunOption)distance(txtOptions.begin(), it); }
        else { cout<<"invalid text option '"<<sw<<"'"<<endl; help(txtOptions); return 0; }
      }
    } // end if('-r')
    else { cout<<"invalid option '"<<argv[i]<<"'"<<endl; help(txtOptions); return 0; }
  } // end for(i)
  
  cout <<"flags:" << endl;
  cout <<"  dbg                      " << dbg         << endl;
  cout <<"  Run Option               "
       <<" "<<txtOptions[ro]<<" ("<<ro<<") "<< endl;
  cout << endl;

  // Create instance of the class:
  FakePlotting* plot = new FakePlotting(ro);
  plot->init();
  plot->setDebug(dbg);

  if(ro == RO_MC)        plot->MCFakeRate();
  if(ro == RO_Data)      plot->DataFakeRate();
  if(ro == RO_DataMCSub) plot->DataRateMCSub();
  if(ro == RO_GJetCR)    plot->GammaJetCRRates();
  if(ro == RO_DataMCSF)  plot->DataMCSF(ro);
  if(ro == RO_DataMC)    plot->DataMCSF(ro);
  if(ro == RO_SRRates)   plot->MCSRRates();
  if(ro == RO_SRComp){   /*plot->Composition();*/ plot->dumpSRTable(); }
  if(ro == RO_TLInfo)    plot->TLPlots();
  if(ro == RO_SRDump)    plot->dumpSRFake();
  if(ro == RO_HFNorm)    plot->GetHFNorm();
  if(ro == RO_DumpPer)   plot->checkPercentages();

  cout << endl;
  cout << "Plotting job done" << endl;

  return 0;
}
