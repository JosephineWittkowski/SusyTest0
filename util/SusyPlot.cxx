
#include <cstdlib>
#include <string>

#include "TChain.h"
#include "Cintex/Cintex.h"

#include "SusyTest0/SusyPlotter.h"
#include "SusyNtuple/ChainHelper.h"

using namespace std;

/*

SusyPlotter - perform selection and dump cutflows 

*/

void help()
{
  cout << "  Options:"                          << endl;
  cout << "  -n number of events to process"    << endl;
  cout << "     defaults: -1 (all events)"      << endl;

  cout << "  -k number of events to skip"       << endl;
  cout << "     defaults: 0"                    << endl;

  cout << "  -d debug printout level"           << endl;
  cout << "     defaults: 0 (quiet) "           << endl;

  cout << "  -F name of single input file"      << endl;
  cout << "     defaults: ''"                   << endl;

  cout << "  -f name of input filelist"         << endl;
  cout << "     defaults: ''"                   << endl;

  cout << "  -D name of input file dir"         << endl;
  cout << "     defaults: ''"                   << endl;

  cout << "  -s sample name, for naming files"  << endl;
  cout << "     defaults: ntuple sample name"   << endl;

  cout << "  --1fb to use the 1/fb mc weights"  << endl;
  cout << "     defualt is false"               << endl;

  cout << "  --AD to use periods A-D weights"   << endl;
  cout << "     default is false;"              << endl;

  cout << "  -h print this help"                << endl;
}

int main(int argc, char** argv)
{
  ROOT::Cintex::Cintex::Enable();

  int nEvt = -1;
  int nSkip = 0;
  int dbg = 0;
  bool use1fb = false;
  bool useAD  = false;
  string sample;
  string file;
  string fileList;
  string fileDir;
  
  cout << "SusyPlotter" << endl;
  cout << endl;

  /** Read inputs to program */
  for(int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0)
      nEvt = atoi(argv[++i]);
    else if (strcmp(argv[i], "-k") == 0)
      nSkip = atoi(argv[++i]);
    else if (strcmp(argv[i], "-d") == 0)
      dbg = atoi(argv[++i]);
    else if (strcmp(argv[i], "-F") == 0)
      file = argv[++i];
    else if (strcmp(argv[i], "-f") == 0)
      fileList = argv[++i];
    else if (strcmp(argv[i], "-D") == 0)
      fileDir = argv[++i];
    else if (strcmp(argv[i], "-s") == 0)
      sample = argv[++i];
    else if (strcmp(argv[i], "--1fb") == 0)
      use1fb = true;
    else if (strcmp(argv[i], "--AD") == 0)
      useAD = true;
    //if (strcmp(argv[i], "-h") == 0)
    else
    {
      help();
      return 0;
    }
  }

  // If no input specified except sample name
  if(file.empty() && fileList.empty() && fileDir.empty() && !sample.empty())
    fileList = "files/" + sample + ".txt";

  // Save the file name
  string fname = "default";
  if( !(fileList.empty()) ){
    int pos = fileList.find(".txt");
    fname = fileList.substr(0,pos);
  }

  cout << "flags:" << endl;
  cout << "  sample  " << sample   << endl;
  cout << "  use 1fb " << use1fb   << endl;
  cout << "  use A-D " << useAD    << endl;
  cout << "  nEvt    " << nEvt     << endl;
  cout << "  nSkip   " << nSkip    << endl;
  cout << "  dbg     " << dbg      << endl;
  if(!file.empty())     cout << "  input   " << file     << endl;
  if(!fileList.empty()) cout << "  input   " << fileList << endl;
  if(!fileDir.empty())  cout << "  input   " << fileDir  << endl;
  cout << "  output  " << fname    << endl;
  cout << endl;

  // Build the input chain
  TChain* chain = new TChain("susyNt");
  ChainHelper::addFile(chain, file);
  ChainHelper::addFileList(chain, fileList);
  ChainHelper::addFileDir(chain, fileDir);
  Long64_t nEntries = chain->GetEntries();
  chain->ls();

  // Build the TSelector
  SusyPlotter* susyPlot = new SusyPlotter();
  susyPlot->setDebug(dbg);
  susyPlot->setSampleName(sample);
  susyPlot->setFileName(fname);
  susyPlot->setUse1fb(use1fb);
  susyPlot->setUseAD(useAD);

  // Run the job
  if(nEvt<0) nEvt = nEntries;
  cout << endl;
  cout << "Total entries:   " << nEntries << endl;
  cout << "Process entries: " << nEvt << endl;
  if(nEvt>0) chain->Process(susyPlot, sample.c_str(), nEvt, nSkip);

  cout << endl;
  cout << "SusyPlotter job done" << endl;

  delete chain;
  return 0;
}
