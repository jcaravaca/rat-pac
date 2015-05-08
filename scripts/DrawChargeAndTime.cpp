#include"/Users/snoplus/tests/jsoncpp/dist/jsoncpp.cpp" //Needs to be included the first
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

#define NPMTs 2
#define USERROOTLOOP true

//Methods
char * gInputFile = NULL;
void ParseArgs(int argc, char **argv);
void GetHistos();

//Globals
std::vector<TH1F*> h_MCPMT_npe; //Number of PE
std::vector<TH1F*> h_MCPMT_charge; //MC charge
std::vector<TH1F*> h_MCPMT_fetime; //MC FE time
std::vector<TH1F*> h_charge; //Measured charge
std::vector<TH1F*> h_time; //Measured time
std::vector<TH1F*> h_time_diff; //Time diff between EV-MC
TH1F* h_charge_total;


int main(int argc, char **argv){

  //Init********
  int appargc = 0;
  char **appargv = NULL;  
  TApplication dummy("App", &appargc, appargv);
  ParseArgs(argc, argv);
  //************

  GetHistos();

  //DRAW PLOTS
  //Charge
  TCanvas *c_charge = new TCanvas("c_charge","c_charge",400*NPMTs,400);
  c_charge->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_charge->cd(ipmt+1);
    h_charge[ipmt]->Draw("");
  }
  TCanvas *c_time = new TCanvas("c_time","c_time",400*NPMTs,400);
  c_time->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_time->cd(ipmt+1);
    h_time[ipmt]->Draw("");
  }
  TCanvas *c_mcfetime = new TCanvas("c_mcfetime","c_mcfetime",400*NPMTs,400);
  c_mcfetime->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_mcfetime->cd(ipmt+1);
    h_MCPMT_fetime[ipmt]->Draw("");
  }
  TCanvas *c_time_diff = new TCanvas("c_time_diff","c_time_diff",400*NPMTs,400);
  c_time_diff->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_time_diff->cd(ipmt+1);
    h_time_diff[ipmt]->Draw("");
  }
  TCanvas *c_npe = new TCanvas("c_npe","c_npe",400*NPMTs,400);
  c_npe->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_npe->cd(ipmt+1);
    h_MCPMT_npe[ipmt]->Draw("");
  }
  
  new TBrowser;
  dummy.Run();
  return 0;

}


void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") {gInputFile = argv[++i]; exist_inputfile=true;}
  }
  if(!exist_inputfile){
    std::cerr<<" Specify input file with option: '-i'"<<std::endl;
    exit(0);
  }
}


void GetHistos(){

  //Init histos
  for(int ih=0; ih<NPMTs; ih++){
    h_MCPMT_npe.push_back(new TH1F(Form("h_mcpmt_npe_%i",ih),"h_mcpmt_npe",10,0,10));
    h_MCPMT_charge.push_back(new TH1F(Form("h_mcpmt_charge_%i",ih),"h_mcpmt_charge",100,0,50));
    h_MCPMT_fetime.push_back(new TH1F(Form("h_MCPMT_fetime_%i",ih),"h_MCPMT_fetime",1000,0,80));
    h_charge.push_back(new TH1F(Form("h_charge_%i",ih),"h_charge",100,0,50));
    h_time.push_back(new TH1F(Form("h_time_%i",ih),"h_time",1000,0,80));
    h_time_diff.push_back(new TH1F(Form("h_time_diff_%i",ih),"h_time_diff",100,-100,100));
  }
  h_charge_total = new TH1F("h_charge_total","h_charge_total",100,0,20);

  //Fill histos with Tree::Draw method (not implemented)
  RAT::DSReader *dsreader = new RAT::DSReader(gInputFile);
  int nentries = dsreader->GetT()->GetEntries();
  std::cout<<" Number of entries: "<<nentries<<std::endl;
  TTree *T = dsreader->GetT();
  for(int ipmt=0; ipmt<NPMTs; ipmt++){
    //EV
    T->Draw(Form("ds.ev.pmt.charge>>h_charge_%i",ipmt),Form("ds.ev.pmt.id==%i",ipmt));
    // T->Draw(Form("ds.ev.pmt.time>>h_time_%i",ipmt),Form("ds.ev.pmt.id==%i",ipmt));
    //MC
    // T->Draw(Form("ds.mc.pmt.GetMCPhotonCount()>>h_mcpmt_npe_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
    // T->Draw(Form("ds.mc.pmt.photon.at(0).frontEndTime>>h_MCPMT_fetime_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
    // T->Draw(Form("ds.mc.pmt.photon.at(0).frontEndTime - ds.ev.pmt.time >> h_time_diff_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
  }
  
  //Fill histos with loop
  if(USERROOTLOOP){
    RAT::DSReader *dsreader = new RAT::DSReader(gInputFile);
    int nentries = dsreader->GetT()->GetEntries();
    std::cout<<" Number of entries: "<<nentries<<std::endl;
    for(int ientry=0; ientry<nentries;++ientry){
      
      if(ientry%(10000) == 0) std::cout<<" Entry "<<ientry<<std::endl;
      RAT::DS::Root *rds = dsreader->GetEvent(ientry);
      RAT::DS::MC *mc = rds->GetMC();
      if(mc->GetMCPMTCount()==0) continue;
      //    if(ientry>1000000) break;
      
      //MC**************
      //MCPMT loop
      for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
	RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
	int pmtid = mcpmt->GetID();
	for (int iph=0; iph < mcpmt->GetMCPhotonCount(); iph++){
	  h_MCPMT_charge[pmtid]->Fill(mcpmt->GetMCPhoton(iph)->GetCharge());
	  h_MCPMT_fetime[pmtid]->Fill(mcpmt->GetMCPhoton(iph)->GetFrontEndTime());
	}
      } //end MCPMT loop
      
      
      //DAQ EVENTS*****
      //Event loop
      int nevents = rds->GetEVCount();
      for(int ievt=0; ievt<nevents; ievt++){
	RAT::DS::EV *ev = rds->GetEV(ievt);
	
	double totalcharge = 0.;
	double charge = 0.;
	for(int ipmt=0; ipmt<ev->GetPMTCount(); ipmt++){
	  int pmtid = ev->GetPMT(ipmt)->GetID();
	  charge = ev->GetPMT(ipmt)->GetCharge();
	//h_charge[pmtid]->Fill(charge);
	  totalcharge += charge;
	  //count PE
	  RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);
	  h_MCPMT_npe[pmtid]->Fill(mcpmt->GetMCPhotonCount());
	}
	if(totalcharge!=0){
	  h_charge_total->Fill(totalcharge);
	}
      } //end daq event loop
    }
  } //end ds entry loop

  
}
