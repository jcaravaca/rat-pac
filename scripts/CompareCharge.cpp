#include<iostream>
#include<fstream>

#include<TH1F.h>
#include<TH2F.h>
#include<TFile.h>
#include<TTree.h>
#include<TCanvas.h>
#include<TApplication.h>
#include<TBrowser.h>
#include<TGraph.h>

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

std::map<int, char*> fInputFiles;
char* fInputData;
void ParseArgs(int argc, char **argv);

int main(int argc, char **argv){

  int appargc = 0;
  char **appargv = NULL;
  TApplication dummy("App", &appargc, appargv);
  ParseArgs(argc, argv);

  RAT::DSReader *dsreader;
  std::vector<TH1F*> hq;
  TH1F *htemp;
  for(int ifile=0; ifile<fInputFiles.size(); ifile++){
    dsreader = new RAT::DSReader(fInputFiles[ifile]);
    htemp = new TH1F(Form("hq%i",ifile),"Charge",300,0,100);
    dsreader->GetT()->Draw(Form("ds.ev.pmt.charge>>hq%i",ifile));
    htemp->Scale(1./htemp->Integral());
    hq.push_back(htemp);
  }

  TH1F *hsum = (TH1F*)hq[0]->Clone();
  for(int ifile=0; ifile<fInputFiles.size(); ifile++)
    hsum->Add(hq[ifile]);
  double max = hsum->GetMaximum();
  
  TGraph* gdata = new TGraph(fInputData,"%lg %lg",",");
  double norm = gdata->Integral(-100,100)*2.5;
  double x=0.;
  double y=0.;
  for(int ip=0; ip<gdata->GetN(); ip++){
    gdata->GetPoint(ip,x,y);
    gdata->SetPoint(ip,x*1.6,y/norm);
  }
  
  TCanvas *c_charge = new TCanvas("c_charge","c_charge",600,600);
  c_charge->cd();
  c_charge->cd()->SetLogy();
  gdata->SetLineColor(kBlue);
  gdata->GetXaxis()->SetTitle("Q(pC)");
  gdata->GetYaxis()->SetRangeUser(1.e-7,1.);
  gdata->Draw();
  hsum->SetLineColor(1);
  hsum->Draw("same");
  for(int ifile=0; ifile<fInputFiles.size(); ifile++){
    hq[ifile]->SetLineColor(ifile+2);
    hq[ifile]->Draw("same");
  }

  
  dummy.Run();
  return 0;

}

void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  bool exist_datafile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") {fInputFiles[0] = argv[++i]; exist_inputfile=true;}
    if(std::string(argv[i]) == "-j") {fInputFiles[1] = argv[++i]; exist_inputfile=true;}
    if(std::string(argv[i]) == "-d") {fInputData = argv[++i]; exist_datafile=true;}
  }

  if(!exist_inputfile){
    std::cerr<<" Specify input mc file with option: '-i', '-j' or '-d'"<<std::endl;
    exit(0);
  }
}
