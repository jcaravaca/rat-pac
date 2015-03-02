#include<iostream>
#include<fstream>

#include<TH1F.h>
#include<TFile.h>
#include<TTree.h>
#include<TCanvas.h>
#include<TApplication.h>
#include<TBrowser.h>
#include<TPolyLine3D.h>
#include<TGraph.h>
#include<TGraph2D.h>
#include<TVector3.h>
#include<TSystem.h>
#include<TMarker.h>
#include<TMath.h>
#include<TGeoBBox.h>

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

#define DEBUG false
#define DRAWEVENT true
#define XCOOR 762.0 // 500.//762.0, 
#define YCOOR 762.0 // 250.//762.0
#define ZCOOR 508.0 // 250.//508.0



char *fInputFile = NULL;
int fEvent = 0;
void ParseArgs(int argc, char **argv);


// class Track{
  
// public:
//   Track(RAT::DS::MCTrack *mctrack);
//   ~Track();

// protected:

//   int pdgcode;
//   int mom;
//   std::vector<double>step_x;
//   std::vector<double>step_y;
//   std::vector<double>step_z;
  
  
// };

// Track::Track(RAT::DS::MCTrack *mctrack){



// };

class EventDisplay{
public:
  EventDisplay(char*);
  ~EventDisplay(){};
  void OpenFile(char*);
  int GetNEvents(){return nevents;};
  void DisplayEvent(int);
  void LoadEvent(int);
  void DumpEventInfo(int);
protected:
  
  std::map<int,Color_t> ParticleColor;
  RAT::DSReader *dsreader;
  RAT::DS::Root *rds;
  RAT::DS::MC *mc;
  int nevents;
  std::vector<TGraph2D> tracks;
  std::vector<TPolyLine3D> pl_tracks;
  std::vector<TGraph> tracks_xz;
  std::vector<TGraph> tracks_xy;
  std::vector<TGraph> tracks_yz;
  std::vector<TGraph> PMTWaveforms;
  std::vector<TGraph> PMTDigitizedWaveforms;
  std::map< int, std::vector<int> > vPMTDigitizedWaveforms;
  TCanvas *canvas_event;

  double elength;
  std::map<int, int> npe; //number of photoelectrons per PMT
  int nelectrons;
  int ncherenkovphotons;
  int notherphotons;
  int nmuons;
  int nothers;
  int ipstart;
  int ipcherenkov;
  int ipbrems;
  int ipphoto;
  int ipeioni;
  int ipothers;
  int epstart;
  int epcherenkov;
  int epbrems;
  int epphoto;
  int epeioni;
  int epatt;
  int G4Fast;
  int epothers;

};


EventDisplay::EventDisplay(char *_inputfile){
  
  OpenFile(_inputfile);
  canvas_event = new TCanvas("canvas_event", "Event", 800, 800);
  canvas_event->Divide(2,2);

  //Set pdg-color mapping
  ParticleColor[11]=kGreen;
  ParticleColor[22]=kRed;
  ParticleColor[13]=kOrange;
  ParticleColor[211]=kOrange;
  ParticleColor[0]=kBlue;

  //Init
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
  nmuons=0;
  nothers=0;
  elength = 0.;
  ipstart = 0;
  ipcherenkov = 0;
  ipbrems = 0;
  ipphoto = 0;
  ipeioni = 0;
  ipothers = 0;
  epstart = 0;
  epcherenkov = 0;
  epbrems = 0;
  epphoto = 0;
  epeioni = 0;
  epatt = 0;
  G4Fast = 0;
  epothers = 0;
   
};

void EventDisplay::OpenFile(char *_inputfile){

  std::cout<<" Opening file "<<_inputfile<<std::endl;  
  dsreader = new RAT::DSReader(_inputfile);
  nevents = dsreader->GetT()->GetEntries();
  
};


void EventDisplay::LoadEvent(int ievt){

  std::cout<<"Loading event "<<ievt<<"......."<<std::endl;

  //Initialize
  //Objects
  rds = dsreader->GetEvent(ievt);
  mc = rds->GetMC();
  //Event features
  elength=0.; //e- lenght
  //Particles
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
  nmuons=0;
  nothers=0;
  //Processes
  ipstart = 0;
  ipcherenkov = 0;
  ipbrems = 0;
  ipphoto = 0;
  ipeioni = 0;
  ipothers = 0;
  epstart = 0;
  epcherenkov = 0;
  epbrems = 0;
  epphoto = 0;
  epeioni = 0;
  epatt = 0;
  G4Fast = 0;
  epothers = 0;

  //Tracks
  tracks.clear();
  pl_tracks.clear();
  tracks_xz.clear();
  tracks_xy.clear();
  tracks_yz.clear();

  //  if(DRAWEVENT){

  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++){
    npe[mc->GetMCPMT(ipmt)->GetID()] = mc->GetMCPMT(ipmt)->GetMCPhotonCount();
  }
  
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
      
      RAT::DS::MCTrack *mctrack = mc->GetMCTrack(itr);
      //Create new track
      tracks.resize(tracks.size()+1);
      pl_tracks.resize(pl_tracks.size()+1);
      tracks_xz.resize(tracks_xz.size()+1);
      tracks_xy.resize(tracks_xy.size()+1);
      tracks_yz.resize(tracks_yz.size()+1);
      //Set PDGcode color code
      tracks.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      pl_tracks.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_xz.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_xy.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_yz.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      //Measure electron length
      if(mctrack->GetPDGCode()==11) elength += mctrack->GetLength();
      //Count particles
      if(mctrack->GetPDGCode()==11) nelectrons++;
      else if(mctrack->GetPDGCode()==0) ncherenkovphotons++;
      else if(mctrack->GetPDGCode()==22) notherphotons++;
      else if(mctrack->GetPDGCode()==13) nmuons++;
      else nothers++;
      //Count processes
      RAT::DS::MCTrackStep *firststep = mctrack->GetMCTrackStep(0);
      RAT::DS::MCTrackStep *laststep = mctrack->GetLastMCTrackStep();
      double last_pos[3];
      laststep->GetEndpoint().GetXYZ(last_pos);
      //      if (itr>=10 && itr<11) std::cout<<" Last pos: "<<last_pos[0]<<" "<<last_pos[1]<<" "<<last_pos[2]<<" "<<std::endl;
      if(firststep->GetProcess()=="Cerenkov") ipcherenkov++;
      else if(firststep->GetProcess()=="start") ipstart++;
      else if(firststep->GetProcess()=="eBrem") ipbrems++;
      else if(firststep->GetProcess()=="eIoni") ipeioni++;
      else if(firststep->GetProcess()=="phot") ipphoto++;
      else {
	ipothers++;
	if (DEBUG) std::cout<<" First proc: "<<firststep->GetProcess().c_str()<<std::endl;
      }
      if(laststep->GetProcess()=="Cerenkov") epcherenkov++;
      else if(laststep->GetProcess()=="start") epstart++;
      else if(laststep->GetProcess()=="eBrem") epbrems++;
      else if(laststep->GetProcess()=="eIoni") epeioni++;
      else if(laststep->GetProcess()=="phot") epphoto++;
      else if(laststep->GetProcess()=="Attenuation") epatt++;
      else if(laststep->GetProcess()=="G4FastSimulationManagerProcess") G4Fast++;
      else {
	epothers++;
	if (DEBUG) std::cout<<" Last proc: "<<laststep->GetProcess().c_str()<<std::endl;
      }
      
      //Loop over all the steps
      int nsteps = mctrack->GetMCTrackStepCount();
      for (int istep = 0; istep < nsteps; istep++) {
	
	RAT::DS::MCTrackStep *step = mctrack->GetMCTrackStep(istep);
	const TVector3 *endpointstep = &step->GetEndpoint();
	double temp_pos[3];
	endpointstep->GetXYZ(temp_pos);
	tracks.back().SetPoint(istep,temp_pos[0],temp_pos[1],temp_pos[2]);
	pl_tracks.back().SetPoint(istep,temp_pos[0],temp_pos[1],temp_pos[2]);
	tracks_xz.back().SetPoint(istep,temp_pos[0],temp_pos[2]);
	tracks_xy.back().SetPoint(istep,temp_pos[0],temp_pos[1]);
	tracks_yz.back().SetPoint(istep,temp_pos[1],temp_pos[2]);

      } //end step loop
      
    } //end track loop
    
    //} end if
    
  PMTWaveforms.clear();
  PMTDigitizedWaveforms.clear();
  PMTDigitizedWaveforms.resize(mc->GetMCPMTCount());

  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
    RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);
    PMTWaveforms.push_back(mcpmt->GetWaveform()->GetGraph());
    vPMTDigitizedWaveforms[ipmt] = mcpmt->GetDigitizedWaveform();

    //Set digitized graphs
    for(int isample=0; isample<vPMTDigitizedWaveforms[ipmt].size(); isample++)
      PMTDigitizedWaveforms[ipmt].SetPoint(isample,isample,vPMTDigitizedWaveforms[ipmt][isample]);
      //      std::cout<<isample<<" "<<vPMTDigitizedWaveforms[ipmt][isample]<<std::endl;
    


  }



  
  if(DEBUG) std::cout<<"LOADED! "<<std::endl;

}



void EventDisplay::DisplayEvent(int ievt){

  LoadEvent(ievt);

  if(DRAWEVENT){
    // //Draw tracks
    // canvas_event->cd(1);
    // tracks_xz[0].Draw("AL");
    // tracks_xz[0].GetXaxis()->SetLimits(-XCOOR,XCOOR);
    // tracks_xz[0].GetYaxis()->SetRangeUser(-ZCOOR,ZCOOR);
    // tracks_xz[0].GetXaxis()->SetTitle("X");
    // tracks_xz[0].GetYaxis()->SetTitle("Z");
    // for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    //   tracks_xz[itr].Draw("LINE same");
    // }
    // canvas_event->cd(2);
    // tracks_yz[0].Draw("AL");
    // tracks_yz[0].GetXaxis()->SetLimits(-YCOOR,YCOOR);
    // tracks_yz[0].GetYaxis()->SetRangeUser(-ZCOOR,ZCOOR);
    // tracks_yz[0].GetXaxis()->SetTitle("Z");
    // tracks_yz[0].GetYaxis()->SetTitle("Y");
    // for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    //   tracks_yz[itr].Draw("LINE same");
    // }
    // canvas_event->cd(3);
    // tracks_xy[0].Draw("AL");
    // tracks_xy[0].GetXaxis()->SetLimits(-XCOOR,XCOOR);
    // tracks_xy[0].GetYaxis()->SetRangeUser(-YCOOR,YCOOR);
    // tracks_xy[0].GetXaxis()->SetTitle("X");
    // tracks_xy[0].GetYaxis()->SetTitle("Y");
    // for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    //   tracks_xy[itr].Draw("LINE same");
    // }
    canvas_event->cd(1)->SetPad(0.,0.3,1.,1.);
    canvas_event->cd(2)->SetPad(0.99,0.99,1.,1.);
    canvas_event->cd(3)->SetPad(0.,0.,0.5,0.3);
    canvas_event->cd(4)->SetPad(0.5,0.,1.,0.3);
    canvas_event->cd(1);
    double pos_temp[] = {0,0,150.0};
    TGeoBBox *bworld = new TGeoBBox(300,300,100, pos_temp);
    bworld->Draw("");
    pos_temp[0] = 0; pos_temp[1] = 0; pos_temp[2] = 200.0;
    TGeoBBox *btarget = new TGeoBBox(20.0,20.0,10.0,pos_temp);
    btarget->Draw("same");
    std::vector<TGeoBBox*> bpmt; bpmt.resize(16);
    for(int ipmt=0; ipmt<bpmt.size(); ipmt++){
      pos_temp[0] = 75.0-50.0*(ipmt%4); pos_temp[1] = 75.0-50.0*(ipmt/4); pos_temp[2] = 100.0;
      bpmt[ipmt] = new TGeoBBox(25.4/2.,25.4/2.,25.4/2.,pos_temp);
      bpmt[ipmt]->Draw("same");
    }
    pl_tracks[0].Draw("LINE");
    // tracks[0].GetXaxis()->SetLimits(-XCOOR,XCOOR);
    // tracks[0].GetYaxis()->SetLimits(-YCOOR,YCOOR);
    // tracks[0].GetZaxis()->SetRangeUser(-ZCOOR,ZCOOR);
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
      //for (int itr = 10; itr < 11; itr++) {
      pl_tracks[itr].Draw("LINE same");
    }
  }
  
  if(mc->GetMCPMTCount()>0){
    //    canvas_event->cd(6)->SetPad(0.99,0.99,1.,1.);
    canvas_event->cd(3);
    //    canvas_event->cd(5)->SetPad(0,0,1.,0.25);
    PMTWaveforms[0].GetXaxis()->SetLimits(0.,50.);
    PMTWaveforms[0].Draw("AP");
    PMTWaveforms[0].GetXaxis()->SetTitle("t(ns)");
    PMTWaveforms[0].GetYaxis()->SetTitle("V");
    double ymax=0.;
    double ymin=9999999.;
    double ymax_temp=0.;
    double ymin_temp=0.;//dummy
    double xmax_temp=0.;//dummy
    double xmin_temp=0.;//dummy
    for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
      PMTWaveforms[ipmt].SetLineColor(ipmt+1);
      PMTWaveforms[ipmt].Draw("LINE same");
      PMTWaveforms[ipmt].ComputeRange(xmin_temp,xmax_temp,ymin_temp,ymax_temp);
      ymax = TMath::Max(ymax,ymax_temp);
      ymin = TMath::Min(ymin,ymin_temp);
    }
    PMTWaveforms[0].GetYaxis()->SetRangeUser(ymin,0.6*ymax);

    canvas_event->cd(4);
    PMTDigitizedWaveforms[0].Draw("AP");
    PMTDigitizedWaveforms[0].GetXaxis()->SetTitle("sample");
    PMTDigitizedWaveforms[0].GetYaxis()->SetTitle("ADC counts");
    for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
      PMTDigitizedWaveforms[ipmt].SetLineColor(ipmt+1);
      PMTDigitizedWaveforms[ipmt].Draw("LINE same");
      PMTDigitizedWaveforms[ipmt].ComputeRange(xmin_temp,xmax_temp,ymin_temp,ymax_temp);
      ymax = TMath::Max(ymax,ymax_temp);
    }
    PMTDigitizedWaveforms[0].GetYaxis()->SetRangeUser(ymin,1.1*ymax);

  }



  
  //Wait for user action
  canvas_event->Modified();
  canvas_event->Update();
  DumpEventInfo(ievt);
  canvas_event->WaitPrimitive();


}

void EventDisplay::DumpEventInfo(int ievt){

  std::cout<<"********EVENT "<<ievt<<"********"<<std::endl;
    std::cout<<"Number of PE"<<std::endl;
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++)
    std::cout<<"ID: "<<ipmt<<" -> "<<npe[ipmt]<<std::endl;
  std::cout<<"Electron lenght: "<<elength<<" mm"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"    INITIAL PROCESSES   "<<std::endl;
  std::cout<<"Number of Start: "<<ipstart<<std::endl;
  std::cout<<"Number of Cherenkov: "<<ipcherenkov<<std::endl;
  std::cout<<"Number of Bremsstrahlung: "<<ipbrems<<std::endl;
  std::cout<<"Number of Photoelectric: "<<ipphoto<<std::endl;
  std::cout<<"Number of e- ionization: "<<ipeioni<<std::endl;
  std::cout<<"Number of others: "<<ipothers<<std::endl;
  std::cout<<std::endl;
  std::cout<<"    END PROCESSES   "<<std::endl;
  std::cout<<"Number of Start: "<<epstart<<std::endl;
  std::cout<<"Number of Cherenkov: "<<epcherenkov<<std::endl;
  std::cout<<"Number of Bremsstrahlung: "<<epbrems<<std::endl;
  std::cout<<"Number of Photoelectric: "<<epphoto<<std::endl;
  std::cout<<"Number of e- ionization: "<<epeioni<<std::endl;
  std::cout<<"Number of attenuation: "<<epatt<<std::endl;
  std::cout<<"Number of others: "<<epothers<<std::endl;
  std::cout<<std::endl;
  std::cout<<"        TRACKS        "<<std::endl;
  std::cout<<"Number of tracks: "<<tracks.size()<<std::endl;
  std::cout<<"Number of electrons: "<<nelectrons<<std::endl;
  std::cout<<"Number of cherenkov photons: "<<ncherenkovphotons<<std::endl;
  std::cout<<"Number of photons: "<<notherphotons<<std::endl;
  std::cout<<"Number of muons: "<<nmuons<<std::endl;
  std::cout<<"Number of others: "<<nothers<<std::endl;
  std::cout<<std::endl;
  std::cout<<"        WAVEFORMS      "<<std::endl;
  std::cout<<" Number of Waveforms "<<mc->GetMCPMTCount()<<std::endl;
  std::cout<<"***********************************"<<std::endl;
  std::cout<<std::endl;
  std::cout<<std::endl;
  std::cout<<" Press any key to go to next event "<<std::endl;

}


int main(int argc, char **argv){

  //Init
  //  gSystem->Load("libGeom");
  int appargc = 0;
  char **appargv = NULL;
  TApplication dummy_app("App", &appargc, appargv);
  ParseArgs(argc, argv);
  EventDisplay *ed = new EventDisplay(fInputFile);

  //Display events
  int nevents = ed->GetNEvents();
  std::cout<<" Number of events: "<<nevents<<std::endl;
  int dummy_val = 0;
  
  for(int ievt=fEvent; ievt<nevents ; ievt++){
    ed->DisplayEvent(ievt);
  }
  
  dummy_app.Run();
  return 0;

  
}


void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") {fInputFile = argv[++i]; exist_inputfile=true;}
    if(std::string(argv[i]) == "-e") {fEvent = std::stoi(argv[++i]);}
  }
  if(!exist_inputfile){
    std::cerr<<" Specify input file with option: '-i'"<<std::endl;
    exit(0);
  }
  
}
