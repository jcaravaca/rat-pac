#include<iostream>
#include<fstream>
#include<string>

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
#include<TGeoManager.h>
#include<TGeoMaterial.h>
#include<TGeoMedium.h>
#include<TGeoVolume.h>

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

#define DEBUG false
#define DRAWPMTS false
#define DRAWOLDDARKBOX true
#define XSIDE 762.0 // 500.//762.0, 
#define YSIDE 762.0 // 250.//762.0
#define ZSIDE 508.0 // 250.//508.0
#define XPOS 0.0
#define YPOS 0.0
#define ZPOS 0.0



char *fInputFile = NULL;
int fEvent = 0;
char *fOpt = "foo";
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
  void DrawGeometry();
  bool IsCerenkov();
  bool IsPE();

protected:
  
  std::map<int,Color_t> ParticleColor;
  std::map<int,int> ParticleWidth;
  RAT::DSReader *dsreader;
  RAT::DS::Root *rds;
  RAT::DS::MC *mc;
  int nevents;
  std::map<int,bool> hitpmts;
  std::vector<TPolyLine3D> pl_tracks;
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

  //Geometry
  TGeoVolume *vworld;
  std::map<int, TGeoBBox* > bpmt;
  std::map<int, TGeoVolume* > vpmt;

};


EventDisplay::EventDisplay(char *_inputfile){
  
  OpenFile(_inputfile);
  //Set canvas
  canvas_event = new TCanvas("canvas_event", "Event", 800, 800);
  canvas_event->Divide(2,2);
  canvas_event->cd(1)->SetPad(0.,0.3,1.,1.);
  canvas_event->cd(2)->SetPad(0.99,0.99,1.,1.);
  canvas_event->cd(3)->SetPad(0.,0.,0.5,0.3);
  canvas_event->cd(4)->SetPad(0.5,0.,1.,0.3);

  //Particle track mapping
  ParticleColor[11]=kGreen;
  ParticleColor[22]=kRed;
  ParticleColor[13]=kOrange;
  ParticleColor[211]=kOrange;
  ParticleColor[0]=kBlue;

  ParticleWidth[11]=1;
  ParticleWidth[22]=1;
  ParticleWidth[13]=2;
  ParticleWidth[211]=2;
  ParticleWidth[0]=1;

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

  //Geometry
  new TGeoManager("box", "poza1");
  TGeoMaterial *mat = new TGeoMaterial("Al", 26.98,13,2.7);
  TGeoMedium *med = new TGeoMedium("MED",1,mat);
  
  double pos_temp[] = {XPOS,YPOS,ZPOS};
  TGeoBBox *bworld = new TGeoBBox(XSIDE,YSIDE,ZSIDE, pos_temp);
  vworld = new TGeoVolume("world",bworld,med);
  //  vworld->SetLineColor(0);
  gGeoManager->SetTopVolume(vworld);
  TGeoBBox *btarget;
  if(DRAWOLDDARKBOX){
    // pos_temp[0] = -180.0; pos_temp[1] = 0.0; pos_temp[2] = 0.0;
    // btarget = new TGeoBBox(2.5,66.7,47.6,pos_temp);
    pos_temp[0] = -450.0; pos_temp[1] = 0.0; pos_temp[2] = -300.0;
    btarget = new TGeoBBox(32.0,100.0,50.0,pos_temp);
  } else{
    pos_temp[0] = 0; pos_temp[1] = 0; pos_temp[2] = 200.0;
    btarget = new TGeoBBox(20.0,20.0,10.0,pos_temp);
  }
  TGeoVolume *vtarget = new TGeoVolume("target",btarget,med);
  vtarget->SetLineWidth(3);
  vtarget->SetLineColor(kCyan);
  vworld->AddNode(vtarget,1);
  if(DRAWPMTS){
    for(int pmtID=0; pmtID<16; pmtID++){
      pos_temp[0] = 75.0-50.0*(pmtID%4); pos_temp[1] = 75.0-50.0*(pmtID/4); pos_temp[2] = 100.0;
      bpmt[pmtID] = new TGeoBBox(25.4/2.,25.4/2.,25.4/2.,pos_temp);
      vpmt[pmtID] = new TGeoVolume(Form("PMT%i",pmtID),bpmt[pmtID],med);
      vpmt[pmtID]->SetLineWidth(2);
      vworld->AddNode(vpmt[pmtID],1);
    }
  }
  
};

void EventDisplay::OpenFile(char *_inputfile){

  std::cout<<" Opening file "<<_inputfile<<std::endl;  
  dsreader = new RAT::DSReader(_inputfile);
  nevents = dsreader->GetT()->GetEntries();
  
};


void EventDisplay::LoadEvent(int ievt){

  if(DEBUG) std::cout<<"Loading event "<<ievt<<"......."<<std::endl;

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
  pl_tracks.clear();
  PMTWaveforms.clear();
  PMTDigitizedWaveforms.clear();


  //Load tracks
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
    
    RAT::DS::MCTrack *mctrack = mc->GetMCTrack(itr);
    //Create new track
    pl_tracks.resize(pl_tracks.size()+1);
    //Set PDGcode color code
    pl_tracks.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
    pl_tracks.back().SetLineWidth(ParticleWidth[mctrack->GetPDGCode()]);
    //Measure electron length
    if(mctrack->GetPDGCode()==11) elength += mctrack->GetLength();
    //Count particles
    if(mctrack->GetPDGCode()==11) nelectrons++;
    else if(mctrack->GetPDGCode()==0) ncherenkovphotons++;
    else if(mctrack->GetPDGCode()==22) notherphotons++;
    else if(mctrack->GetPDGCode()==13) nmuons++;
    else {
      nothers++;
      if (DEBUG) std::cout<<" Unknown particle: "<<mctrack->GetPDGCode()<<std::endl;
    }
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
      if (DEBUG) std::cout<<" Unknown first proc: "<<firststep->GetProcess().c_str()<<std::endl;
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
      if (DEBUG) std::cout<<" Unknown last proc: "<<laststep->GetProcess().c_str()<<std::endl;
    }
    
    //Loop over all the steps
    int nsteps = mctrack->GetMCTrackStepCount();
    for (int istep = 0; istep < nsteps; istep++) {
      
      RAT::DS::MCTrackStep *step = mctrack->GetMCTrackStep(istep);
      const TVector3 *endpointstep = &step->GetEndpoint();
      double temp_pos[3];
      endpointstep->GetXYZ(temp_pos);
      pl_tracks.back().SetPoint(istep,temp_pos[0],temp_pos[1],temp_pos[2]);
      
    } //end step loop
    
  } //end track loop

  
  //Load photoelectrons
  for (int ipmt = 0; ipmt < 16; ipmt++)
    hitpmts[ipmt] = false; //clear
  
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++){
    int pmtID = mc->GetMCPMT(ipmt)->GetID();
    npe[pmtID] = mc->GetMCPMT(ipmt)->GetMCPhotonCount();
    hitpmts[pmtID] = false;
    if(npe[pmtID] != 0) hitpmts[pmtID] = true;
  }

  
  //Load waveforms
  PMTDigitizedWaveforms.resize(mc->GetMCPMTCount());
  int ymax_d=0.; //yaxis max limit digital
  int ymin_d=9999999.; //yaxis min limit digital
  double ymax=0.; //yaxis max limit analogue
  double ymin=9999999.; //yaxis min limit analogue
  double ymax_temp=0.;
  double ymin_temp=0.;
  double xmax_temp=0.;//dummy
  double xmin_temp=0.;//dummy
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
    RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);
    PMTWaveforms.push_back(mcpmt->GetWaveform()->GetGraph());
    vPMTDigitizedWaveforms[ipmt] = mcpmt->GetDigitizedWaveform();
    //Compute graph limits
    PMTWaveforms[ipmt].ComputeRange(xmin_temp,ymin_temp,xmax_temp,ymax_temp);
    ymax = TMath::Max(ymax,ymax_temp);
    ymin = TMath::Min(ymin,ymin_temp);
    ymax = (ymax == 0)? 0.1:ymax;

    //Set digitized graphs
    for(int isample=0; isample<vPMTDigitizedWaveforms[ipmt].size(); isample++){
      PMTDigitizedWaveforms[ipmt].SetPoint(isample,isample*2.0,(vPMTDigitizedWaveforms[ipmt][isample] - 8200.)/330.);
      //      std::cout<<isample<<" "<<vPMTDigitizedWaveforms[ipmt][isample]<<std::endl;
      ymax_d = TMath::Max(ymax_d,vPMTDigitizedWaveforms[ipmt][isample]);
      ymin_d = TMath::Min(ymin_d,vPMTDigitizedWaveforms[ipmt][isample]);
    }
    
    if(DEBUG) std::cout<<" analogue limits "<<ymax<<" "<<ymin<<std::endl;
    if(DEBUG) std::cout<<" digital limits "<<ymax_d<<" "<<ymin_d<<std::endl;
    //    PMTWaveforms[ipmt].GetYaxis()->SetRangeUser(2.0*ymin,1.2*ymax);
    PMTWaveforms[ipmt].GetYaxis()->SetRangeUser(-1.5,0.5);
    PMTDigitizedWaveforms[ipmt].GetYaxis()->SetRangeUser(0.99*ymin_d,1.01*ymax_d);

  }
  
  if(DEBUG) std::cout<<"LOADED! "<<std::endl;

}

//Draw experiment geometry in canvas
void EventDisplay::DrawGeometry(){

  canvas_event->cd(1);

  //Highlight PMT if was hit
  if(DRAWPMTS){
    for(int pmtID=0; pmtID<16; pmtID++){
      vpmt[pmtID]->SetLineColor(1);
      if(hitpmts[pmtID]) vpmt[pmtID]->SetLineColor(kRed);
    }
  }
  vworld->Draw("");
  //  vworld->Draw("ogl");
  
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
  std::cout<<"Number of tracks: "<<pl_tracks.size()<<std::endl;
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
  std::cout<<" Press any key to go to next event "<<std::endl;

}


void EventDisplay::DisplayEvent(int ievt){

  if(DEBUG) std::cout<<"Display canvas 1 "<<std::endl;

  canvas_event->cd(1);
  DrawGeometry();
  pl_tracks[0].Draw("LINE");
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
    pl_tracks[itr].Draw("LINE same");
  }
  
  if(mc->GetMCPMTCount()>0){

    if(DEBUG) std::cout<<"Display canvas 2 "<<std::endl;

    canvas_event->cd(3);
    PMTWaveforms[0].GetXaxis()->SetLimits(0.,50.);
    PMTWaveforms[0].Draw("AP");
    PMTWaveforms[0].GetXaxis()->SetTitle("t(ns)");
    PMTWaveforms[0].GetYaxis()->SetTitle("V");
    for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
      PMTWaveforms[ipmt].SetLineColor(ipmt+1);
      PMTWaveforms[ipmt].Draw("LINE same");
      PMTDigitizedWaveforms[ipmt].SetLineColor(kRed);
      PMTDigitizedWaveforms[ipmt].Draw("LINE same");
    }

    if(DEBUG) std::cout<<"Display canvas 3 "<<std::endl;

    canvas_event->cd(4);
    PMTDigitizedWaveforms[0].Draw("AP");
    PMTDigitizedWaveforms[0].GetXaxis()->SetTitle("sample");
    PMTDigitizedWaveforms[0].GetYaxis()->SetTitle("ADC counts");
    for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
      //      PMTDigitizedWaveforms[ipmt].SetLineColor(ipmt+1);
      PMTDigitizedWaveforms[ipmt].Draw("LINE same");
      //      PMTDigitizedWaveforms[ipmt].ComputeRange(xmin_temp,xmax_temp,ymin_temp,ymax_temp);
    }

  }

  
  //Wait for user action
  canvas_event->Modified();
  canvas_event->Update();
  canvas_event->WaitPrimitive();


}

bool EventDisplay::IsCerenkov(){

  return ncherenkovphotons>0;
  
}

bool EventDisplay::IsPE(){

  int npe_total = 0;
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++)
    npe_total += npe[ipmt];

  return npe_total>0;
  
}

int main(int argc, char **argv){

  //Init
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
    ed->LoadEvent(ievt);
    if(std::string(fOpt) == "cerenkov" && !ed->IsCerenkov()) continue;
    if(std::string(fOpt) == "pe" && !ed->IsPE()) continue;
    if(DEBUG) std::cout<<" After Cerenkov Check "<<std::endl;
    ed->DumpEventInfo(ievt);
    if(DEBUG) std::cout<<" After Dump Event "<<std::endl;
    ed->DisplayEvent(ievt);
    if(DEBUG) std::cout<<" After Display Event "<<std::endl;
  }
  
  dummy_app.Run();
  return 0;

  
}


void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") {fInputFile = argv[++i]; exist_inputfile=true;}
    if(std::string(argv[i]) == "-e") {fEvent = std::stoi(argv[++i]);}
    if(std::string(argv[i]) == "-o") {fOpt = argv[++i];}
  }
  if(!exist_inputfile){
    std::cerr<<" Specify input file with option: '-i'"<<std::endl;
    exit(0);
  }
  if(DEBUG){
    std::cout<<" Input file "<<fInputFile<<std::endl;
    std::cout<<" Event "<<fEvent<<std::endl;
    std::cout<<" Option "<<fOpt<<std::endl;
  }

    
}
