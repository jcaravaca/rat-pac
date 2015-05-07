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
#define USEROOTLOOP true

//Methods
char * gInputFileMC = NULL;
std::vector<char*> gInputFileDT;
void ParseArgs(int argc, char **argv);
void GetHistos();
void NormalizeHistos();

//Globals
std::vector<TH1F*> h_mcpmt_npe; //Number of PE
std::vector<TH1F*> h_mcpmt_charge; //MC charge
std::vector<TH1F*> h_mcpmt_fetime; //MC FE time
std::vector<TH1F*> h_charge; //Measured charge
std::vector<TH1F*> h_time; //Measured time
std::vector<TH1F*> h_time_diff; //Time diff between EV-MC
std::vector<TH2F*> h_charge_scat; //PMT charge vs trigger charge
TH1F* h_charge_total;

//Real data
std::vector<TH1F*> h_dt_charge; //Measured charge

int main(int argc, char **argv){

  //Init********
  int appargc = 0;
  char **appargv = NULL;  
  TApplication dummy("App", &appargc, appargv);
  ParseArgs(argc, argv);
  //************

  GetHistos();
  if(gInputFileDT.size()>0)
    NormalizeHistos();
  
  //DRAW PLOTS
  //Charge
  TCanvas *c_charge = new TCanvas("c_charge","c_charge",400*NPMTs,400);
  c_charge->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_charge->cd(ipmt+1);
    h_charge[ipmt]->Draw("");
  }
  if(gInputFileDT.size()>0){
    c_charge->cd(1);
    h_dt_charge[0]->SetLineColor(kRed);
    h_dt_charge[0]->Draw("same");
  }
  TCanvas *c_charge_scat = new TCanvas("c_charge_scat","c_charge_scat",400*NPMTs,400);
  c_charge_scat->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_charge_scat->cd(ipmt+1);
    h_charge_scat[ipmt]->Draw("colz");
  }
  TCanvas *c_mccharge = new TCanvas("c_mccharge","c_mccharge",400*NPMTs,400);
  c_mccharge->Divide(NPMTs,1);
  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    c_mccharge->cd(ipmt+1);
    h_mcpmt_charge[ipmt]->Draw("");
  }
  if(gInputFileDT.size()>0){
    c_mccharge->cd(1);
    h_dt_charge[0]->Draw("same");
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
    h_mcpmt_fetime[ipmt]->Draw("");
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
    h_mcpmt_npe[ipmt]->Draw("");
  }
  
  new TBrowser;
  dummy.Run();
  return 0;

}


void GetHistos(){

  //MC
  std::cout<<" GetMCPDFs "<<std::endl;
  //Init histos
  for(int ih=0; ih<NPMTs; ih++){
    h_mcpmt_npe.push_back(new TH1F(Form("h_mcpmt_npe_%i",ih),"h_mcpmt_npe",10,0,10));
    h_mcpmt_charge.push_back(new TH1F(Form("h_mcpmt_charge_%i",ih),"h_mcpmt_charge",200,0,100));
    h_mcpmt_fetime.push_back(new TH1F(Form("h_mcpmt_fetime_%i",ih),"h_mcpmt_fetime",1000,0,80));
    h_charge.push_back(new TH1F(Form("h_charge_%i",ih),"h_charge",200,0,100));
    h_charge_scat.push_back(new TH2F(Form("h_charge_scat_%i",ih),"h_charge_scat",200,0,100,200,0,100));
    h_time.push_back(new TH1F(Form("h_time_%i",ih),"h_time",1000,0,80));
    h_time_diff.push_back(new TH1F(Form("h_time_diff_%i",ih),"h_time_diff",100,-100,100));
  }
  h_charge_total = new TH1F("h_charge_total","h_charge_total",100,0,20);

  //Fill histos with loop
  if(USEROOTLOOP){

    RAT::DSReader *dsreader = new RAT::DSReader(gInputFileMC);
    TTree *tree = dsreader->GetT();
    int nentries = tree->GetEntries();
    std::cout<<" Number of entries: "<<nentries<<std::endl;
    for(int ientry=0; ientry<nentries;++ientry){
      
      if(ientry%(10000) == 0) std::cout<<" Entry "<<ientry<<std::endl;
      //    if(ientry>1000000) break;
      tree->GetEntry(ientry);
      RAT::DS::Root *rds = dsreader->GetEvent(ientry);
      RAT::DS::MC *mc = rds->GetMC();
      //If no PMT continue to save time (in theory)
      if(mc->GetMCPMTCount()==0) continue;
      
      //MC**************
      //MCPMT loop
      for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
      	RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
      	int pmtid = mcpmt->GetID();
      	for (int iph=0; iph < mcpmt->GetMCPhotonCount(); iph++){
      	  h_mcpmt_charge[pmtid]->Fill(mcpmt->GetMCPhoton(iph)->GetCharge());
      	  h_mcpmt_fetime[pmtid]->Fill(mcpmt->GetMCPhoton(iph)->GetFrontEndTime());
      	}
      } //end MCPMT loop
      
      
      //DAQ EVENTS*****
      //Event loop
      int nevents = rds->GetEVCount();
      for(int ievt=0; ievt<nevents; ievt++){
	RAT::DS::EV *ev = rds->GetEV(ievt);
	double totalcharge = 0.;
	double charge = 0.;
	double charge_trig = 0.;
	//Look for trigger PMT and save charge
	for(int ipmt=0; ipmt<ev->GetPMTCount(); ipmt++){
	  int trigid = ev->GetPMT(ipmt)->GetID();
	  if(trigid==1){
	    charge_trig = ev->GetPMT(ipmt)->GetCharge();
	    break;
	  }
	}
	for(int ipmt=0; ipmt<ev->GetPMTCount(); ipmt++){
	  int pmtid = ev->GetPMT(ipmt)->GetID();
	  charge = ev->GetPMT(ipmt)->GetCharge();
	  h_charge[pmtid]->Fill(charge);
	  h_charge_scat[pmtid]->Fill(charge,charge_trig); //charge vs trigger charge
	  h_time[pmtid]->Fill(ev->GetPMT(ipmt)->GetTime());
	  totalcharge += charge;
	  //count PE
	  RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);
	  h_mcpmt_npe[pmtid]->Fill(mcpmt->GetMCPhotonCount());
	}
	if(totalcharge!=0){
	  h_charge_total->Fill(totalcharge);
	}
      } //end daq event loop
    } //end ds entry loop

  } else { //if !USERROOTLOOP

    //Fill histos with Tree::Draw method
    RAT::DSReader *dsreader = new RAT::DSReader(gInputFileMC);
    int nentries = dsreader->GetT()->GetEntries();
    std::cout<<" Number of entries: "<<nentries<<std::endl;
    TTree *T = dsreader->GetT();
    for(int ipmt=0; ipmt<NPMTs; ipmt++){
      //EV
      std::cout<<"   Loading charge... "<<std::endl;
      T->Draw(Form("ds.ev.pmt.charge>>h_charge_%i",ipmt),Form("ds.ev.pmt.id==%i",ipmt));
      std::cout<<"   Loading time... "<<std::endl;
      T->Draw(Form("ds.ev.pmt.time>>h_time_%i",ipmt),Form("ds.ev.pmt.id==%i",ipmt));
      //MC
      // std::cout<<"   Loading mc charge... "<<std::endl;
      // T->Draw(Form("ds.mc.pmt.GetCharge()>>h_mcpmt_charge_%i",ipmt),Form("ds.mc.pmt.id==%i",ipmt));
      // T->Draw(Form("ds.mc.pmt.GetMCPhotonCount()>>h_mcpmt_npe_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
      // T->Draw(Form("ds.mc.pmt.photon.at(0).frontEndTime>>h_mcpmt_fetime_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
      // T->Draw(Form("ds.mc.pmt.photon.at(0).frontEndTime - ds.ev.pmt.time >> h_time_diff_%i",ipmt),Form("ds.ev.Nhits()>0 && ds.mc.pmt.id==%i",ipmt));
    }
  }
  
  //REAL DATA
  if(gInputFileDT.size()>0){ 
    std::cout<<" GetDataPDFs "<<std::endl;
    
    TGraph* gpdf_dt;
    TH1F* hdata;
    TH1F* hscale;
    for(int ifile=0; ifile<gInputFileDT.size(); ifile++){
      std::cout<<" gInputFileDT "<<ifile<<" "<<gInputFileDT[ifile]<<std::endl;
      hdata = new TH1F(Form("hdata%i",ifile),"hdata",200,0,100);
      hscale = new TH1F(Form("hscale%i",ifile),"hscale",200,0,100); //count how many times we fill the bin and scale it, root is anoying...
      gpdf_dt = new TGraph(gInputFileDT[ifile],"%lg %lg",",");
      double x=0.;
      double y=0.;
      for(int ip=0; ip<gpdf_dt->GetN(); ip++){
	gpdf_dt->GetPoint(ip,x,y);
	hdata->Fill(x,y);
	hscale->Fill(x,1.);
      }
      hdata->Divide(hscale);
      //    hdata->Scale(1./hdata->Integral()); //Nomalize for shape only analysis
      h_dt_charge.push_back(hdata);
    }
  }
  

}

void NormalizeHistos(){

  for(int ipmt=0; ipmt<NPMTs;ipmt++){
    double norm = h_charge[ipmt]->Integral(21,100);
    h_charge[ipmt]->Scale(1./norm);
    std::cout<<" norm "<<ipmt<<" "<<norm<<std::endl;
    norm = h_mcpmt_charge[ipmt]->Integral(21,100);
    h_mcpmt_charge[ipmt]->Scale(1./norm);
    std::cout<<" norm "<<ipmt<<" "<<norm<<std::endl;
  }
  
  if(gInputFileDT.size()>0){
    double norm = h_dt_charge[0]->Integral(21,100);
    h_dt_charge[0]->Scale(1./norm);
  }
  
}


void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-mc") {gInputFileMC = argv[++i]; exist_inputfile=true;}
    if(std::string(argv[i]) == "-dt") {gInputFileDT.push_back(argv[++i]); exist_inputfile=true;}
  }

  if(!exist_inputfile){
    std::cerr<<" Usage: ./DrawChargeAndTime.exe -mc MCFILE [Optional: -dt DATAFILE]"<<std::endl;
    exit(0);
  }
}
