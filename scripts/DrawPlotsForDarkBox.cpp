#include"/Users/snoplus/tests/jsoncpp/dist/jsoncpp.cpp" //Needs to be included the first
#include<iostream>
#include<fstream>

#include<TH1F.h>
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

#define NORM_ATT 7.0

// std::vector<std::string> processes;

// void Init(){

//   processes.push_back("start");
//   processes.push_back("eIoni");
//   processes.push_back("Cerenkov");
//   processes.push_back("Attenuation");
//   processes.push_back("eBrem");
//   processes.push_back("phot");
//   processes.push_back("Transportation");
//   processes.push_back("msc");
//   processes.push_back("G4FastSimulationManagerProcess");
//   processes.push_back("compt");
//   processes.push_back("Reemission");
  
// }


// int parse_process(std::string name){

//   for(int iproc=0; iproc<processes.size(); iproc++){
//     if(name==processes[iproc]) return iproc;
//     //std::cout<< name <<" "<< processes[iproc] <<std::endl;
//   }
  
//   //If no return
//   std::cerr<<" Process "<< name << " not found" << std::endl;
//   exit(0);
  
// }


char * fInputFile = NULL;
void ParseArgs(int argc, char **argv);


int main(int argc, char **argv){

  int appargc = 0;
  char **appargv = NULL;
  
  TApplication dummy("App", &appargc, appargv);


  ParseArgs(argc, argv);
  
  TH1F* h_MCPMT_charge = new TH1F("h_MCPMT_charge","h_MCPMT_charge",100,0,20);
  TH1F* h_procinit = new TH1F("h_procinit","h_procinit",1,0,1);
  TH1F* h_proclast = new TH1F("h_proclast","h_proclast",1,0,1);
  TH1F* h_procinit_gm = new TH1F("h_procinit_gm","h_procinit_gm",1,0,1);
  TH1F* h_proclast_gm = new TH1F("h_proclast_gm","h_proclast_gm",1,0,1);
  TH1F* h_MCPMT_isdh = new TH1F("h_MCPMT_isdh","h_MCPMT_isdh",2,0,2);
  TH1F* h_MCPMT_time = new TH1F("h_MCPMT_time","h_MCPMT_time",100,1,2);
  TH1F* h_ntracks = new TH1F("h_ntracks","h_ntracks",100,0,5000);
  TH1F* h_ncp = new TH1F("h_ncp","h_ncp",100,0,1000);
  TH1F* h_npe = new TH1F("h_npe","h_npe",10,0,10);
  TH1F* h_cp_length = new TH1F("h_cp_length","h_cp_length",300,0,6000);
  TH1F* h_cp_ke = new TH1F("h_cp_ke","h_cp_ke",300,0,3e-5);
  TH1F* h_cp_wl = new TH1F("h_cp_wl","h_cp_wl",300,0,1000);
  TH1F* h_ph_last = new TH1F("h_ph_last","h_ph_last",500,-4000,4000);

  //  Init();

  RAT::DSReader *dsreader = new RAT::DSReader(fInputFile);
  RAT::DS::Root *rds = dsreader->GetDS();
  TTree *t = dsreader->GetT();

  int nentries = t->GetEntries();
  std::cout<<" Number of entries: "<<nentries<<std::endl;


  //Init event loop
  for (int ievt = 0; ievt < nentries; ievt++) {

    if(ievt%(100) == 0) std::cout<<" Entry "<<ievt<<std::endl;
    
    t->GetEntry(ievt);
    RAT::DS::MC *mc = rds->GetMC();
    //    RAT::DS::EV *ev = rds->GetEV(0);

    //Init PMT loop
    //    int npmts = ev->GetPMTCount();
    //    std::cout<<" Number of PMTs in Event "<<ievt<<": "<<npmts<<std::endl;
    
    // for (int ipmt = 0; ipmt < npmts; ipmt++) {
    //   RAT::DS::PMT *pmt = ev->GetPMT(ipmt);
    // }
    
    int ntracks = mc->GetMCTrackCount();
    int ncerphotons = 0; //number of cerenkov photons
    h_ntracks->Fill(ntracks);
    //    std::cout<<" Number of tracks in Event "<<ievt<<": "<<ntracks<<std::endl;

    //Init track loop
    for (int itr = 0; itr < ntracks; itr++) {
      
      RAT::DS::MCTrack *track = mc->GetMCTrack(itr);
      RAT::DS::MCTrackStep *f_step = track->GetMCTrackStep(0);
      RAT::DS::MCTrackStep *l_step = track->GetLastMCTrackStep();

      // int f_step_parsed = parse_process(f_step->GetProcess());
      // int l_step_parsed = parse_process(l_step->GetProcess());

      // std::cout<<" F_Step Process: "<<f_step->GetProcess()<<" "<<f_step_parsed<<std::endl;
      // std::cout<<" Last Process: "<<l_step->GetProcess()<<" "<<last_parsed<<std::endl;

      // h_procinit->Fill(f_step_parsed);
      // h_proclast->Fill(l_step_parsed);

      //Fill histograms
      h_procinit->Fill(f_step->GetProcess().c_str(),1.);
      h_proclast->Fill(l_step->GetProcess().c_str(),1.);

      if(track->GetPDGCode() == 22 || track->GetPDGCode() == 0){ //gamma or optical photon
	h_procinit_gm->Fill(f_step->GetProcess().c_str(),1.);
	h_proclast_gm->Fill(l_step->GetProcess().c_str(),1.);
	h_ph_last->Fill(l_step->GetEndpoint().x());
	if(f_step->GetProcess() == "Cerenkov"){
	  ncerphotons++;
	  h_cp_length->Fill(track->GetLength());
	  h_cp_ke->Fill(f_step->GetKE());
	  h_cp_wl->Fill(1.24/(f_step->GetKE()*1.e3));
	}
      }
    }//end track loop
    h_ncp->Fill(ncerphotons);

    //Init MCPMT loop
    h_npe->Fill(mc->GetNumPE());
    for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
      RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
      for (int i=0; i < mcpmt->GetMCPhotonCount(); i++){
	h_MCPMT_isdh->Fill(mcpmt->GetMCPhoton(i)->IsDarkHit());
	h_MCPMT_charge->Fill(mcpmt->GetMCPhoton(i)->GetCharge());
	h_MCPMT_time->Fill(mcpmt->GetMCPhoton(i)->GetHitTime());
	// std::cout<<" IsDarkHit "<< i << mcpmt->GetMCPhoton(i)->isDarkHit <<std::endl;	
      }
      
    }
    
  }//end entry loop
  

  //Get attenuation from json file
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;

  std::ifstream att("acry_att_spectrum.json");
  bool parsingSuccessful = reader.parse( att, root, false );
  if ( !parsingSuccessful ){
    // report to the user the failure and their locations in the document.
    std::cout  << reader.getFormatedErrorMessages()
  	       << "\n";
  }
  
  const Json::Value json_att_x = root["ABSLENGTH_value1"];
  const Json::Value json_att_y = root["ABSLENGTH_value2"];
  const int npoints_att = json_att_x.size();
    
  //Convert to array
  double vatt_x[npoints_att];
  for ( int index = 0; index < npoints_att; ++index ){
    vatt_x[index] = json_att_x[index].asDouble();
    //   std::cout<<att_x[index]<<std::endl;
  }
  double vatt_y[npoints_att];
  for ( int index = 0; index < npoints_att; ++index ){
    vatt_y[index] = NORM_ATT*json_att_y[index].asDouble();
    //    std::cout<<vatt_y[index]<<std::endl;
  }

  TGraph *gAtt = new TGraph(npoints_att,vatt_x,vatt_y);



  
  //DRAW PLOTS
  //Charge
  TCanvas *c_charge = new TCanvas("c_charge","c_charge",1400,600);
  c_charge->Divide(2,1);
  c_charge->cd(1);
  t->Draw("ds.ev.pmt.charge>>test(100,0,20)");
  c_charge->cd(2);
  h_MCPMT_charge->Draw();
  //Process
  TCanvas *c_process = new TCanvas("c_process","c_process",1400,600);
  c_process->Divide(2,1);
  c_process->cd(1);
  h_procinit->Draw();
  c_process->cd(2);
  h_proclast->Draw();
  TCanvas *c_process_gm = new TCanvas("c_process_gm","c_process_gm",1400,600);
  c_process_gm->Divide(2,1);
  c_process_gm->cd(1);
  h_procinit_gm->Draw();
  c_process_gm->cd(2);
  h_proclast_gm->Draw();
  //IsDarkHit
  TCanvas *c_isdh = new TCanvas("c_isdh","c_isdh",600,600);
  c_isdh->cd();
  h_MCPMT_isdh->Draw();
  //Time
  TCanvas *c_time = new TCanvas("c_time","c_time",600,600);
  c_time->cd();
  h_MCPMT_time->Draw();
  //NTracks
  TCanvas *c_ntracks = new TCanvas("c_ntracks","c_ntracks",1400,600);
  c_ntracks->Divide(2,1);
  c_ntracks->cd(1);
  h_ntracks->Draw();
  c_ntracks->cd(2);
  //Cerenkov photons
  TCanvas *c_cp = new TCanvas("c_cp","c_cp",1400,1400);
  c_cp->Divide(2,2);
  c_cp->cd(1);
  h_ncp->Draw();
  c_cp->cd(2);
  h_cp_length->GetYaxis()->SetRangeUser(0.,600.);
  h_cp_length->Draw();
  c_cp->cd(3);
  h_cp_wl->Draw();
  gAtt->Draw("same");
  //  h_npe->Draw();
  //  h_cp_ke->Draw();
  c_cp->cd(4);
  h_ph_last->Draw();


  //DRAW TABLE
  double ph_total = h_cp_length->GetEntries();
  double ph_att = h_cp_length->Integral(0,(int)100.*300./6000.);
  double ph_hitpmt = h_cp_length->Integral((int)600.*300./6000.,(int)800.*300./6000.);
  double ph_elec = h_MCPMT_isdh->GetEntries();
  std::cout<<" # Cherenkov photons: "<<ph_total<<" (in "<<ph_total/nentries<<" per event)"<<std::endl;
  std::cout<<" # photons attenuated: "<<ph_att<<" ("<<ph_att/ph_total*100<<"\%)"<<std::endl;
  std::cout<<" # photons hitting PC: "<<ph_hitpmt<<" ("<<ph_hitpmt/ph_total*100<<"\%)"<<std::endl;
  std::cout<<" # photo-electrons: "<<ph_elec<<" ("<<ph_elec/ph_hitpmt*100<<"\%)"<<std::endl;
  std::cout<<" 0 photo-electrons: "<<h_npe->GetBinContent(1)<<" ("<<h_npe->GetBinContent(1)/nentries*100<<"\%)"<<std::endl;
  std::cout<<" 1 photo-electrons: "<<h_npe->GetBinContent(2)<<" ("<<h_npe->GetBinContent(2)/nentries*100<<"\%)"<<std::endl;
  std::cout<<" Multi photo-electrons: "<<h_npe->Integral(3,10)<<" ("<<h_npe->Integral(3,10)/nentries*100<<"\%)"<<std::endl;
  
  new TBrowser;
  dummy.Run();
  return 0;

}


void ParseArgs(int argc, char **argv){
  //  int nargs = 1;
  // if(argc<(nargs*2+1))
  //   exit(1);
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") fInputFile = argv[++i];
  }
}
