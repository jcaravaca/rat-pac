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

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>


char *fInputFile = NULL;
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
  void DumpEventInfo();
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
  TCanvas *canvas;
  int nelectrons;
  int ncherenkovphotons;
  int notherphotons;
  int nothers;
  
};


EventDisplay::EventDisplay(char *_inputfile){
  
  OpenFile(_inputfile);
  canvas = new TCanvas("canvas", "test", 800, 800);
  canvas->Divide(2,2);

  //Set pdg-color mapping
  ParticleColor[11]=kGreen;
  ParticleColor[22]=kBlue;
  ParticleColor[0]=kBlue;

  //Init
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
  nothers=0;

};

void EventDisplay::OpenFile(char *_inputfile){

  std::cout<<" Opening file "<<_inputfile<<std::endl;  
  dsreader = new RAT::DSReader(_inputfile);
  nevents = dsreader->GetT()->GetEntries();
  
};


void EventDisplay::LoadEvent(int ievt){

  std::cout<<"Loading event "<<ievt<<"......."<<std::endl;

  rds = dsreader->GetEvent(ievt);
  mc = rds->GetMC();
  nelectrons=0;
  ncherenkovphotons=0;
  notherphotons=0;
  nothers=0;
  tracks.clear();
  tracks_xz.clear();
  tracks_xy.clear();
  tracks_yz.clear();

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
    //Set track info
    if(mctrack->GetPDGCode()==11) nelectrons++;
    else if(mctrack->GetPDGCode()==0) ncherenkovphotons++;
    else if(mctrack->GetPDGCode()==22) notherphotons++;
    else nothers++;

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



void EventDisplay::DisplayEvent(int ievt){

  LoadEvent(ievt);

  //Draw tracks
  canvas->cd(1);
  tracks_xz[0].Draw("AL");
  tracks_xz[0].GetXaxis()->SetLimits(-762.0,762.0);
  tracks_xz[0].GetYaxis()->SetRangeUser(-508.0,508.0);
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    tracks_xz[itr].Draw("LINE same");
  }
  canvas->cd(2);
  tracks_yz[0].Draw("AL");
  tracks_yz[0].GetXaxis()->SetLimits(-762.0,762.0);
  tracks_yz[0].GetYaxis()->SetRangeUser(-508.0,508.0);
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    tracks_yz[itr].Draw("LINE same");
  }
  canvas->cd(3);
  tracks_xy[0].Draw("AL");
  tracks_xy[0].GetXaxis()->SetLimits(-762.0,762.0);
  tracks_xy[0].GetYaxis()->SetRangeUser(-762.0,762.0);
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++){
    tracks_xy[itr].Draw("LINE same");
  }
  canvas->cd(4);
  tracks[0].Draw("LINE");
  tracks[0].GetXaxis()->SetLimits(-762.0,762.0);
  tracks[0].GetYaxis()->SetLimits(-762.0,762.0);
  tracks[0].GetZaxis()->SetRangeUser(-508.0,508.0);

  //  tracks[0].SetMaximum
  for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
    tracks[itr].Draw("LINE same");
  }

  //Wait for user action
  canvas->Modified();
  canvas->Update();
  std::cout<<"Showing event "<<ievt<<std::endl;
  DumpEventInfo();
  std::cout<<" Press any key to go to next event "<<std::endl;
  canvas->WaitPrimitive();


}

void EventDisplay::DumpEventInfo(){

  std::cout<<"********TRACKS*******"<<std::endl;
  std::cout<<"Number of tracks: "<<tracks.size()<<std::endl;
  std::cout<<"Number of electrons: "<<nelectrons<<std::endl;
  std::cout<<"Number of cherenkov photons: "<<ncherenkovphotons<<std::endl;
  std::cout<<"Number of photons: "<<notherphotons<<std::endl;
  std::cout<<"Number of nothers: "<<nothers<<std::endl;

  
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

  for(int ievt=0; ievt<nevents ; ievt++){
    ed->DisplayEvent(ievt);
  }

  dummy_app.Run();
  return 0;

  
}


void ParseArgs(int argc, char **argv){
  bool exist_inputfile = false;
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-i") {fInputFile = argv[++i]; exist_inputfile=true;}
  }
  if(!exist_inputfile){
    std::cerr<<" Specify input file with option: '-i'"<<std::endl;
    exit(0);
  }
}
