#include<iostream>
#include<fstream>
#include<string>

#include<TH1F.h>
#include<TH2F.h>
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
#include<TStyle.h>
#include<TMarker.h>
#include<TMath.h>
#include<TGeoBBox.h>
#include<TGeoManager.h>
#include<TGeoMaterial.h>
#include<TGeoMedium.h>
#include<TGeoVolume.h>
#include<TLine.h>
#include<TBox.h>
#include<TPaveText.h>

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

#define DEBUG false
#define DRAWTRACKS true
#define DRAWWAVEFORMS false
#define DRAWPMTS false
#define DRAWOLDDARKBOX false

//Geometry
#define DB_XSIDE 762.0 // 500.//762.0, 
#define DB_YSIDE 762.0 // 250.//762.0
#define DB_ZSIDE 508.0 // 250.//508.0
#define XPOS 0.0
#define YPOS 0.0
#define ZPOS 0.0
#define XP_XSIDE 200.0 // 250.//508.0
#define XP_YSIDE 200.0 // 250.//508.0
#define XP_ZPOS 110.0 // 116.0



char *fInputFile = NULL;
int fEvent = -9999;
char *fOpt = "NULL";
void ParseArgs(int argc, char **argv);


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
  
  //Maps
  std::map<std::string,int> FirstProcessCounter; //Process name - Counter
  std::map<std::string,int> LastProcessCounter; //Process name - Counter
  std::map<int,std::string> ParticleName; //PDG code - Particle name
  std::map<std::string,int> ParticleCounter; //PDG code - Counter
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
  TH2F *hxyplane;
  std::map< int, std::vector<int> > vPMTDigitizedWaveforms;
  TCanvas *canvas_event;

  double elength;
  std::map<int, int> npe; //number of photoelectrons per PMT

  //Geometry
  TGeoVolume *vworld;
  std::map<int, TGeoBBox* > bpmt;
  std::map<int, TGeoVolume* > vpmt;
  double pmtwidth = 30.; //mm
  std::vector<TPaveText> vpmtbox;

};


EventDisplay::EventDisplay(char *_inputfile){
  
  OpenFile(_inputfile);
  //Set canvas
  gStyle->SetGridWidth(1);
  canvas_event = new TCanvas("canvas_event", "Event", 800, 1000);
  canvas_event->Divide(2,2);
  canvas_event->cd(1)->SetPad(0.,0.3,1.,1.);
  canvas_event->cd(2)->SetPad(0.99,0.99,1.,1.);
  canvas_event->cd(3)->SetPad(0.,0.,0.5,0.3);
  canvas_event->cd(4)->SetPad(0.5,0.,1.,0.3);

  //Particle maps
  ParticleColor[11]=kGreen;   ParticleWidth[11]=1;   ParticleName[11]="Electron";
  ParticleColor[22]=kRed;     ParticleWidth[22]=1;   ParticleName[22] = "Standard photon";
  ParticleColor[13]=kOrange;  ParticleWidth[13]=2;   ParticleName[13] = "Muon";
  ParticleColor[211]=kOrange; ParticleWidth[211]=2;  ParticleName[211]= "Pi+";
  ParticleColor[0]=kBlue;     ParticleWidth[0]=1;    ParticleName[0] = "Optical photon";
  
  //Representation plane
  hxyplane = new TH2F("hxyplane","Track intersections with XY plane",1000,-XP_XSIDE,XP_XSIDE,1000,-XP_YSIDE,XP_YSIDE);
  
  //Geometry
  new TGeoManager("box", "poza1");
  TGeoMaterial *mat = new TGeoMaterial("Al", 26.98,13,2.7);
  TGeoMedium *med = new TGeoMedium("MED",1,mat);
  
  double pos_temp[] = {XPOS,YPOS,ZPOS};
  TGeoBBox *bworld = new TGeoBBox(DB_XSIDE,DB_YSIDE,DB_ZSIDE, pos_temp);
  vworld = new TGeoVolume("world",bworld,med);
  gGeoManager->SetTopVolume(vworld);
  TGeoBBox *bdarkbox;
  bdarkbox = new TGeoBBox(DB_XSIDE,DB_YSIDE,DB_ZSIDE,pos_temp);
  TGeoVolume *vdarkbox = new TGeoVolume("darkbox",bdarkbox,med);
  vdarkbox->SetLineWidth(3);
  vdarkbox->SetLineColor(1);
  vworld->AddNode(vdarkbox,1);
  TGeoBBox *btarget;
  if(DRAWOLDDARKBOX){
    // pos_temp[0] = -180.0; pos_temp[1] = 0.0; pos_temp[2] = 0.0;
    // btarget = new TGeoBBox(2.5,66.7,47.6,pos_temp);
    pos_temp[0] = -450.0; pos_temp[1] = 0.0; pos_temp[2] = -300.0;
    btarget = new TGeoBBox(32.0,100.0,50.0,pos_temp);
  } else{
    pos_temp[0] = 0; pos_temp[1] = 0; pos_temp[2] = 200.0;
    btarget = new TGeoBBox(20.0,20.0,1.0,pos_temp);
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

  //PMTs in XY plane
  //x pmts
  for(int ipmt=0;ipmt<7;ipmt++){
    vpmtbox.push_back(TPaveText(pmtwidth*ipmt-3*pmtwidth-pmtwidth/2.,-15.,pmtwidth*ipmt-2*pmtwidth-pmtwidth/2.,15.));
  }
  //y pmts
  for(int ipmt=0;ipmt<7;ipmt++){
    if(ipmt==3) continue; //do not draw the central PMT again
    vpmtbox.push_back(TPaveText(-15.,pmtwidth*ipmt-3*pmtwidth-pmtwidth/2.,15.,pmtwidth*ipmt-2*pmtwidth-pmtwidth/2.));
  }
  //offset pmts
  vpmtbox.push_back(TPaveText(pmtwidth*5-3*pmtwidth-pmtwidth/2.,pmtwidth*5-3*pmtwidth-pmtwidth/2.,pmtwidth*5-2*pmtwidth-pmtwidth/2.,pmtwidth*5-2*pmtwidth-pmtwidth/2.));
  vpmtbox.push_back(TPaveText(pmtwidth*1-3*pmtwidth-pmtwidth/2.,pmtwidth*1-3*pmtwidth-pmtwidth/2.,pmtwidth*1-2*pmtwidth-pmtwidth/2.,pmtwidth*1-2*pmtwidth-pmtwidth/2.));
  //customize
  for(int ipmt=0;ipmt<vpmtbox.size();ipmt++){
    vpmtbox[ipmt].SetFillColor(kGray);
    vpmtbox[ipmt].SetFillStyle(3004);
    vpmtbox[ipmt].SetLineColor(2);
    vpmtbox[ipmt].SetLineWidth(1);
    vpmtbox[ipmt].SetTextColor(1);
    vpmtbox[ipmt].SetTextSize(0.04);
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
  FirstProcessCounter.clear();
  LastProcessCounter.clear();
  ParticleCounter.clear();
  pl_tracks.clear();
  PMTWaveforms.clear();
  PMTDigitizedWaveforms.clear();
  hxyplane->Reset();
  npe.clear();

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
    ParticleCounter[ParticleName[mctrack->GetPDGCode()]] += 1;
    //Count processes
    RAT::DS::MCTrackStep *firststep = mctrack->GetMCTrackStep(0);
    RAT::DS::MCTrackStep *laststep = mctrack->GetLastMCTrackStep();
    double last_pos[3];
    laststep->GetEndpoint().GetXYZ(last_pos);
    //      if (itr>=10 && itr<11) std::cout<<" Last pos: "<<last_pos[0]<<" "<<last_pos[1]<<" "<<last_pos[2]<<" "<<std::endl;
    FirstProcessCounter[firststep->GetProcess()] += 1;
    LastProcessCounter[laststep->GetProcess()] += 1;

    //Loop over all the steps
    int nsteps = mctrack->GetMCTrackStepCount();
    TVector3 top_pos(-9999.,-9999.,-9999.); //first interpolation point
    TVector3 bottom_pos(9999.,9999.,9999.); //second interpolation point
    TVector3 int_pos(9999.,9999.,9999.);//intersection point
    for (int istep = 0; istep < nsteps; istep++) {
      
      RAT::DS::MCTrackStep *step = mctrack->GetMCTrackStep(istep);
      const TVector3 endpointstep = step->GetEndpoint();
      pl_tracks.back().SetPoint(istep,endpointstep.X(),endpointstep.Y(),endpointstep.Z());

      //Calculate intersection with XY plane
      // std::cout<<"step "<<istep<<" "<<top_pos.X()<<" "<<top_pos.Y()<<" "<<top_pos.Z()<<std::endl;
      // std::cout<<"step "<<istep<<" "<<bottom_pos.X()<<" "<<bottom_pos.Y()<<" "<<bottom_pos.Z()<<std::endl;
      if(mctrack->GetPDGCode()!=0 && mctrack->GetPDGCode()!=22) continue; //only for photons
      if(bottom_pos.Z()!=-9999.){ //we haven't found the point yet
	if(endpointstep.Z()>XP_ZPOS){
	  top_pos = endpointstep;
	}
	else if(top_pos.Z()!=-9999.){ //this is our guy
	  bottom_pos = endpointstep;
	  //Intersect!
	  double lambda = (XP_ZPOS - top_pos.Z())/(bottom_pos.Z() - top_pos.Z());
	  int_pos = top_pos + (bottom_pos - top_pos)*lambda;
	  //	  std::cout<<"FILL IT! "<<int_pos.X()<<" "<<int_pos.Y()<<" "<<int_pos.Z()<<std::endl;
	  hxyplane->Fill(int_pos.X(),int_pos.Y());
	  bottom_pos.SetZ(-9999.);
	}
      }
    } //end step loop
  } //end track loop

  
  //Load photoelectrons
  for (int ipmt = 0; ipmt < 16; ipmt++){
    hitpmts[ipmt] = false; //clear
    npe[ipmt]=0;
  }
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++){
    int pmtID = mc->GetMCPMT(ipmt)->GetID();
    npe[pmtID] = mc->GetMCPMT(ipmt)->GetMCPhotonCount();
    hitpmts[pmtID] = false;
    if(npe[pmtID] != 0) hitpmts[pmtID] = true;
  }

  for(int ipmt=0;ipmt<vpmtbox.size();ipmt++){
    vpmtbox[ipmt].Clear();
    vpmtbox[ipmt].AddText(Form("%d",npe[ipmt]));
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
      PMTDigitizedWaveforms[ipmt].SetPoint(isample,isample,vPMTDigitizedWaveforms[ipmt][isample]);
      //      PMTDigitizedWaveforms[ipmt].SetPoint(isample,isample*2.0,(vPMTDigitizedWaveforms[ipmt][isample] - 8200.)/330.);
      //      std::cout<<"Digit waveforms "<<isample<<" "<<vPMTDigitizedWaveforms[ipmt][isample]<<std::endl;
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
  for (std::map<int,int>::iterator it=npe.begin();it!=npe.end();it++){
    if(it==npe.begin()) std::cout<<"Number of PE"<<std::endl;
    if(it->second!=0) std::cout<<"ID: "<<it->first<<" -> "<<it->second<<std::endl;
  }


  // for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++){
  //   if(ipmt==0) std::cout<<"Number of PE"<<std::endl;
  //   std::cout<<"ID: "<<mc->GetMCPMT(ipmt)->GetID()<<" -> "<<npe[ipmt]<<std::endl;
  //   //    std::cout<<"ID: "<<ipmt<<" -> "<<npe[ipmt]<<std::endl;
  // }

  std::cout<<"Electron lenght: "<<elength<<" mm"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"    INITIAL PROCESSES "<<std::endl;
  for (std::map<std::string,int>::iterator it=FirstProcessCounter.begin();it!=FirstProcessCounter.end();it++){
    if(it->second!=0) std::cout<<it->first<<" == "<<it->second<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"    END PROCESSES   "<<std::endl;
  for (std::map<std::string,int>::iterator it=LastProcessCounter.begin();it!=LastProcessCounter.end();it++){
    if(it->second!=0) std::cout<<it->first<<" == "<<it->second<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"    TRACKS        "<<std::endl;
  for (std::map<std::string,int>::iterator it=ParticleCounter.begin();it!=ParticleCounter.end();it++){
    if(it->second!=0) std::cout<<it->first<<" == "<<it->second<<std::endl;
  }
  std::cout<<std::endl;
  std::cout<<"    WAVEFORMS      "<<std::endl;
  std::cout<<" Number of Waveforms "<<mc->GetMCPMTCount()<<std::endl;
  std::cout<<"***********************************"<<std::endl;
  std::cout<<std::endl;
  std::cout<<" Press any key to go to next event "<<std::endl;

}


void EventDisplay::DisplayEvent(int ievt){

  if(DEBUG) std::cout<<"Display canvas 1 "<<std::endl;

  if(DRAWTRACKS){
    canvas_event->cd(1);
    DrawGeometry();
    //    pl_tracks[0].Draw("LINE");
    for (int itr = 0; itr < mc->GetMCTrackCount(); itr++) {
      pl_tracks[itr].Draw("LINE same");
    }
    
  }
  
  if(DRAWWAVEFORMS){
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
	//      PMTDigitizedWaveforms[ipmt].SetLineColor(kRed);
	//      PMTDigitizedWaveforms[ipmt].Draw("LINE same");
      }
      
      if(DEBUG) std::cout<<"Display canvas 3 "<<std::endl;
      
      canvas_event->cd(4);
      PMTDigitizedWaveforms[0].Draw("AP");
      PMTDigitizedWaveforms[0].GetXaxis()->SetTitle("sample");
      PMTDigitizedWaveforms[0].GetYaxis()->SetTitle("ADC counts");
      for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
	PMTDigitizedWaveforms[ipmt].SetLineColor(ipmt+1);
	PMTDigitizedWaveforms[ipmt].Draw("LINE same");
      //      PMTDigitizedWaveforms[ipmt].ComputeRange(xmin_temp,xmax_temp,ymin_temp,ymax_temp);
      }
    }
  } else{
    canvas_event->cd(3);
    hxyplane->Draw("box");
    //Draw grid
    int nlines = 2*XP_XSIDE/pmtwidth;
    TLine* xline;
    TLine* yline;
    for(int iline=0; iline<nlines; iline++){
      xline = new TLine(-XP_XSIDE,iline*pmtwidth-nlines/2.*pmtwidth,XP_XSIDE,iline*pmtwidth-nlines/2.*pmtwidth);
      xline->SetLineWidth(1.);
      xline->SetLineStyle(3);
      xline->SetLineColor(kGray);
      xline->Draw("same");
      yline = new TLine(iline*pmtwidth-nlines/2.*pmtwidth,-XP_YSIDE,iline*pmtwidth-nlines/2.*pmtwidth,XP_YSIDE);
      yline->SetLineWidth(1.);
      yline->SetLineStyle(3);
      yline->SetLineColor(kGray);
      yline->Draw("same");
    }

    //Draw pmts
    for(int ipmt=0; ipmt<vpmtbox.size();ipmt++)
      vpmtbox[ipmt].Draw("LINE same");

  }    

  //Wait for user action
  canvas_event->Modified();
  canvas_event->Update();
  canvas_event->WaitPrimitive();


}

bool EventDisplay::IsCerenkov(){

  return FirstProcessCounter["Cerenkov"]>0;
  
}

bool EventDisplay::IsPE(){

  int npe_total = 0;
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++)
    npe_total += npe[mc->GetMCPMT(ipmt)->GetID()];

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
  
  if(fEvent==-9999){
    for(int ievt=0; ievt<nevents ; ievt++){
      ed->LoadEvent(ievt);
      if(std::string(fOpt) == "cerenkov" && !ed->IsCerenkov()) continue;
      if(std::string(fOpt) == "pe" && !ed->IsPE()) continue;
      if(DEBUG) std::cout<<" After Cerenkov Check "<<std::endl;
      ed->DumpEventInfo(ievt);
      if(DEBUG) std::cout<<" After Dump Event "<<std::endl;
      ed->DisplayEvent(ievt);
      if(DEBUG) std::cout<<" After Display Event "<<std::endl;
    }
  }
  else{
    ed->LoadEvent(fEvent);
    if(DEBUG) std::cout<<" After Cerenkov Check "<<std::endl;
    ed->DumpEventInfo(fEvent);
    if(DEBUG) std::cout<<" After Dump Event "<<std::endl;
    ed->DisplayEvent(fEvent);
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
    std::cerr<<" Usage: EventDisplay.exe -i INPUTFILE [(optional) -e EVNUMBER -o OPTION]"<<std::endl;
    //    std::cerr<<" Specify input file with option: '-i'"<<std::endl;
    exit(0);
  }
  if(DEBUG){
    std::cout<<" Input file "<<fInputFile<<std::endl;
    std::cout<<" Event "<<fEvent<<std::endl;
    std::cout<<" Option "<<fOpt<<std::endl;
  }

    
}
