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

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

#define DEBUG false
#define DRAWEVENT true
#define XCOOR 762.//762.0
#define YCOOR 762.//762.0
#define ZCOOR 508.//508.0



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
  std::vector<TGraph> tracks_xz;
  std::vector<TGraph> tracks_xy;
  std::vector<TGraph> tracks_yz;
  std::vector<TGraph> PMTWaveforms;
  TCanvas *canvas_event;
  int nelectrons;
  int ncherenkovphotons;
  int notherphotons;
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
  int epothers;

};


EventDisplay::EventDisplay(char *_inputfile){
  
  OpenFile(_inputfile);
  canvas_event = new TCanvas("canvas_event", "Event", 800, 800);
  canvas_event->Divide(2,3);

  //Set pdg-color mapping
  ParticleColor[11]=kGreen;
  ParticleColor[22]=kBlue;
  ParticleColor[0]=kBlue;

  //Init
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
  nothers=0;
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
  //Particles
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
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
  epothers = 0;

  //Tracks
  tracks.clear();
  tracks_xz.clear();
  tracks_xy.clear();
  tracks_yz.clear();

  if(DRAWEVENT){

    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
      
      RAT::DS::MCTrack *mctrack = mc->GetMCTrack(itr);
      //Create new track
      tracks.resize(tracks.size()+1);
      tracks_xz.resize(tracks_xz.size()+1);
      tracks_xy.resize(tracks_xy.size()+1);
      tracks_yz.resize(tracks_yz.size()+1);
      //Set PDGcode color code
      tracks.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_xz.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_xy.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      tracks_yz.back().SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
      //Count particles
      if(mctrack->GetPDGCode()==11) nelectrons++;
      else if(mctrack->GetPDGCode()==0) ncherenkovphotons++;
      else if(mctrack->GetPDGCode()==22) notherphotons++;
      else nothers++;
      //Count processes
      RAT::DS::MCTrackStep *firststep = mctrack->GetMCTrackStep(0);
      RAT::DS::MCTrackStep *laststep = mctrack->GetLastMCTrackStep();
      if (DEBUG) std::cout<<firststep->GetProcess().c_str()<<std::endl;
      double last_pos[3];
      laststep->GetEndpoint().GetXYZ(last_pos);
      //      if (itr>=10 && itr<11) std::cout<<" Last pos: "<<last_pos[0]<<" "<<last_pos[1]<<" "<<last_pos[2]<<" "<<std::endl;
      if(firststep->GetProcess()=="Cerenkov") ipcherenkov++;
      else if(firststep->GetProcess()=="start") ipstart++;
      else if(firststep->GetProcess()=="eBrem") ipbrems++;
      else if(firststep->GetProcess()=="eIoni") ipeioni++;
      else if(firststep->GetProcess()=="phot") ipphoto++;
      else ipothers++;
      if(laststep->GetProcess()=="Cerenkov") epcherenkov++;
      else if(laststep->GetProcess()=="start") epstart++;
      else if(laststep->GetProcess()=="eBrem") epbrems++;
      else if(laststep->GetProcess()=="eIoni") epeioni++;
      else if(laststep->GetProcess()=="phot") epphoto++;
      else if(laststep->GetProcess()=="Attenuation") epatt++;
      else epothers++;
	
      //Loop over all the steps
      int nsteps = mctrack->GetMCTrackStepCount();
      for (int istep = 0; istep < nsteps; istep++) {
	
	RAT::DS::MCTrackStep *step = mctrack->GetMCTrackStep(istep);
	const TVector3 *endpointstep = &step->GetEndpoint();
	double temp_pos[3];
	endpointstep->GetXYZ(temp_pos);
	tracks.back().SetPoint(istep,temp_pos[0],temp_pos[1],temp_pos[2]);
	tracks_xz.back().SetPoint(istep,temp_pos[0],temp_pos[2]);
	tracks_xy.back().SetPoint(istep,temp_pos[0],temp_pos[1]);
	tracks_yz.back().SetPoint(istep,temp_pos[1],temp_pos[2]);

      } //end step loop
      
    } //end track loop

  }
  
  PMTWaveforms.clear();

  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
    RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);
    PMTWaveforms.push_back(mcpmt->GetWaveform()->GetGraph());
  }
  
  if(DEBUG) std::cout<<"LOADED! "<<std::endl;

}



void EventDisplay::DisplayEvent(int ievt){

  LoadEvent(ievt);

  if(DRAWEVENT){
    //Draw tracks
    canvas_event->cd(1);
    tracks_xz[0].Draw("AL");
    tracks_xz[0].GetXaxis()->SetLimits(-XCOOR,XCOOR);
    tracks_xz[0].GetYaxis()->SetRangeUser(-ZCOOR,ZCOOR);
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
      tracks_xz[itr].Draw("LINE same");
    }
    canvas_event->cd(2);
    tracks_yz[0].Draw("AL");
    tracks_yz[0].GetXaxis()->SetLimits(-YCOOR,YCOOR);
    tracks_yz[0].GetYaxis()->SetRangeUser(-ZCOOR,ZCOOR);
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
      tracks_yz[itr].Draw("LINE same");
    }
    canvas_event->cd(3);
    tracks_xy[0].Draw("AL");
    tracks_xy[0].GetXaxis()->SetLimits(-XCOOR,XCOOR);
    tracks_xy[0].GetYaxis()->SetRangeUser(-YCOOR,YCOOR);
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
      tracks_xy[itr].Draw("LINE same");
    }
    canvas_event->cd(4);
    tracks[0].Draw("LINE");
    tracks[0].GetXaxis()->SetLimits(-762.0,762.0);
    tracks[0].GetYaxis()->SetLimits(-762.0,762.0);
    tracks[0].GetZaxis()->SetRangeUser(-508.0,508.0);
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
      //for (int itr = 10; itr < 11; itr++) {
      tracks[itr].Draw("LINE same");
    }
  }
  
  if(mc->GetMCPMTCount()>0){
    canvas_event->cd(6)->Delete();
    canvas_event->cd(5);
    canvas_event->cd(5)->SetPad(0,0,1.,0.25);
    PMTWaveforms[0].Draw("AP");
    double ymax=0.;
    double ymax_temp=0.;
    double ymin_temp=0.;//dummmy
    double xmax_temp=0.;//dummmy
    double xmin_temp=0.;//dummmy
    for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
      PMTWaveforms[ipmt].SetLineColor(ipmt+1);
      PMTWaveforms[ipmt].Draw("LINE same");
      PMTWaveforms[ipmt].ComputeRange(xmin_temp,xmax_temp,ymin_temp,ymax_temp);
      ymax = TMath::Max(ymax,ymax_temp);
    }
    PMTWaveforms[0].GetYaxis()->SetRangeUser(-1.,ymax*1.2);
  }
  
  //Wait for user action
  canvas_event->Modified();
  canvas_event->Update();
  DumpEventInfo(ievt);
  canvas_event->WaitPrimitive();


}

void EventDisplay::DumpEventInfo(int ievt){

  std::cout<<"********EVENT "<<ievt<<"********"<<std::endl;
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
