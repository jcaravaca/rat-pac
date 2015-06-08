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
#define DRAWWAVEFORMS false //if false -> Draw rings
#define DRAWPMTS true
#define DRAWOLDDARKBOX false
#define DRAWVESSEL true
#define DRAWXYPLANE true

//Geometry
#define DB_XSIDE 762.0 // 500.//762.0, 
#define DB_YSIDE 762.0 // 250.//762.0
#define DB_ZSIDE 508.0 // 250.//508.0
#define XPOS 0.0
#define YPOS 0.0
#define ZPOS 0.0
#define XP_XSIDE 200.0 // 250.//508.0
#define XP_YSIDE 200.0 // 250.//508.0
#define XP_ZPOS 129.5 // 130.0

double pmtwidth = 29.0; //mm


char *fInputFile = NULL;
int fEvent = -9999;
char *fOpt = "NULL";
int fItrack = 0;
int fFtrack = 9999;
bool fUserSetNtracks = false;
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
  void DumpDisplayInfo();
  void DrawGeometry();
  bool IsCerenkov();
  bool IsPE();
  void CustomizeTrack(TPolyLine3D*,RAT::DS::MCTrack*);

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
  std::map< int, std::vector<double> > vPMTWaveforms;
  std::map< int, std::vector<int> > vPMTDigitizedWaveforms;
  std::map<std::string,TH2F*> hxyplane;
  TCanvas *canvas_event;

  double elength;
  std::map<int, int> npe; //number of photoelectrons per PMT

  //Geometry
  TGeoVolume *vworld;
  std::map<int, TGeoBBox* > bpmt;
  std::map<int, TGeoVolume* > vpmt;
  std::vector<TPaveText> vpmtbox;

};


EventDisplay::EventDisplay(char *_inputfile){
  
  //Init
  OpenFile(_inputfile);

  //Set canvas
  gStyle->SetGridWidth(1);
  canvas_event = new TCanvas("canvas_event", "Event", 600, 1000);
  canvas_event->Divide(2,2);
  canvas_event->cd(1)->SetPad(0.,0.3,1.,1.);
  canvas_event->cd(2)->SetPad(0.99,0.99,1.,1.);
  canvas_event->cd(3)->SetPad(0.,0.,0.5,0.3);
  canvas_event->cd(4)->SetPad(0.5,0.,1.,0.3);

  //Particle maps
  ParticleColor[11]=kGreen;   ParticleWidth[11]=1;   ParticleName[11]="Electron";
  ParticleColor[22]=kYellow;     ParticleWidth[22]=1;   ParticleName[22] = "Standard photon";
  ParticleColor[13]=kOrange;  ParticleWidth[13]=2;   ParticleName[13] = "Muon";
  ParticleColor[211]=kOrange; ParticleWidth[211]=2;  ParticleName[211]= "Pi+";
  ParticleColor[0]=kCyan+1;     ParticleWidth[0]=1;    ParticleName[0] = "Cherenkov photon"; //Indeed this is an optical photon, but I changed the definition
  ParticleColor[9999]=kRed-7;     ParticleWidth[9999]=1;    ParticleName[9999] = "Scintillation photon"; //Created by me, PDG number doesn't actually exist
  
  //Representation plane
  hxyplane["start"] = new TH2F("hxyplane_cher","Track intersections with XY plane: Cherenkov",1000,-XP_XSIDE,XP_XSIDE,1000,-XP_YSIDE,XP_YSIDE);
  hxyplane["Cerenkov"] = new TH2F("hxyplane_cher","Track intersections with XY plane: Cherenkov",1000,-XP_XSIDE,XP_XSIDE,1000,-XP_YSIDE,XP_YSIDE);
  hxyplane["Scintillation"] = new TH2F("hxyplane_scint","Track intersections with XY plane: Scintillation",1000,-XP_XSIDE,XP_XSIDE,1000,-XP_YSIDE,XP_YSIDE);
  
  //Geometry
  TGeoManager *tgeoman = new TGeoManager("box", "poza1");
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

  if(DRAWXYPLANE){
    //XY plane
    TGeoBBox *bxyplane;
    pos_temp[0] = 0.0; pos_temp[1] = 0.0; pos_temp[2] = XP_ZPOS;
    bxyplane = new TGeoBBox(DB_XSIDE/2.,DB_YSIDE/2.,.0001,pos_temp);
    TGeoVolume *vxyplane = new TGeoVolume("xyplane",bxyplane,med);
    vxyplane->SetLineWidth(1);
    vxyplane->SetLineColor(1);
    vworld->AddNode(vxyplane,3);
  }
  TGeoBBox *bvessel;
  TGeoBBox *bcontent;
  if(DRAWOLDDARKBOX){
    // pos_temp[0] = -180.0; pos_temp[1] = 0.0; pos_temp[2] = 0.0;
    // bvessel = new TGeoBBox(2.5,66.7,47.6,pos_temp);
    pos_temp[0] = -450.0; pos_temp[1] = 0.0; pos_temp[2] = -300.0;
    bvessel = new TGeoBBox(32.0,100.0,50.0,pos_temp);
  } else{
    pos_temp[0] = 0; pos_temp[1] = 0; pos_temp[2] = 179.3;
    bvessel = new TGeoBBox(200.0,200.0,50.0,pos_temp);
    bcontent = new TGeoBBox(190.0,190.0,40.0,pos_temp);
  }
  if(DRAWVESSEL){
    TGeoVolume *vvessel = new TGeoVolume("vessel",bvessel,med);
    TGeoVolume *vcontent = new TGeoVolume("content",bcontent,med);
    vvessel->SetLineWidth(3);
    vvessel->SetLineColor(kBlue);
    vcontent->SetLineWidth(2);
    vcontent->SetLineColor(kBlue+1);
    vworld->AddNode(vvessel,1);
    //    vworld->AddNode(vcontent,2);
  }
  if(DRAWPMTS){
    //Grid
    // for(int pmtID=0; pmtID<16; pmtID++){
    //   pos_temp[0] = 75.0-50.0*(pmtID%4); pos_temp[1] = 75.0-50.0*(pmtID/4); pos_temp[2] = 100.0;
    //   bpmt[pmtID] = new TGeoBBox(25.4/2.,25.4/2.,25.4/2.,pos_temp);
    //   vpmt[pmtID] = new TGeoVolume(Form("PMT%i",pmtID),bpmt[pmtID],med);
    //   vpmt[pmtID]->SetLineWidth(2);
    //   vworld->AddNode(vpmt[pmtID],1);
    // }
    //Cross
    double pos_pmts[16][3];
    pos_pmts[0][0] = -90.; pos_pmts[0][1] = 0.; pos_pmts[0][2] = 115.;
    pos_pmts[1][0] = -60.; pos_pmts[1][1] = 0.; pos_pmts[1][2] = 115.;
    pos_pmts[2][0] = -30.; pos_pmts[2][1] = 0.; pos_pmts[2][2] = 115.;
    pos_pmts[3][0] = 0.; pos_pmts[3][1] = 0.; pos_pmts[3][2] = 115.;
    pos_pmts[4][0] = 30.; pos_pmts[4][1] = 0.; pos_pmts[4][2] = 115.;
    pos_pmts[5][0] = 60.; pos_pmts[5][1] = 0.; pos_pmts[5][2] = 115.;
    pos_pmts[6][0] = 90.; pos_pmts[6][1] = 0.; pos_pmts[6][2] = 115.;
    pos_pmts[7][0] = 0.; pos_pmts[7][1] = -90.; pos_pmts[7][2] = 115.;
    pos_pmts[8][0] = 0.; pos_pmts[8][1] = -90.; pos_pmts[8][2] = 115.;
    pos_pmts[9][0] = 0.; pos_pmts[9][1] = -60.; pos_pmts[9][2] = 115.;
    pos_pmts[10][0] = 0.; pos_pmts[10][1] = -30.; pos_pmts[10][2] = 115.;
    pos_pmts[11][0] = 0.; pos_pmts[11][1] = 30.; pos_pmts[11][2] = 115.;
    pos_pmts[12][0] = 0.; pos_pmts[12][1] = 60.; pos_pmts[12][2] = 115.;
    pos_pmts[13][0] = 0.; pos_pmts[13][1] = 90.; pos_pmts[13][2] = 115.;
    pos_pmts[14][0] = 60.; pos_pmts[14][1] = 60.; pos_pmts[14][2] = 115.;
    pos_pmts[15][0] = -60.; pos_pmts[15][1] = -60.; pos_pmts[15][2] = 115.;

    for(int pmtID=0; pmtID<16; pmtID++){
      bpmt[pmtID] = new TGeoBBox(pmtwidth/2.,pmtwidth/2.,pmtwidth/2.,pos_pmts[pmtID]);
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
    vpmtbox[ipmt].SetTextColor(kBlack);
    vpmtbox[ipmt].SetTextSize(0.04);
  }
 
};

void EventDisplay::OpenFile(char *_inputfile){

  std::cout<<" Opening file "<<_inputfile<<std::endl;  
  dsreader = new RAT::DSReader(_inputfile);
  nevents = dsreader->GetT()->GetEntries();
  
};



void EventDisplay::CustomizeTrack(TPolyLine3D *track, RAT::DS::MCTrack *mctrack){

  RAT::DS::MCTrackStep *firststep = mctrack->GetMCTrackStep(0);
  if(firststep->GetProcess()=="Scintillation")
    mctrack->SetPDGCode(9999);
      
  //Set color
  track->SetLineColor(ParticleColor[mctrack->GetPDGCode()]);
  //Set width
  track->SetLineWidth(ParticleWidth[mctrack->GetPDGCode()]);
  //Count particles
  ParticleCounter[ParticleName[mctrack->GetPDGCode()]] += 1;

}

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
  for (std::map<std::string,TH2F*>::iterator it=hxyplane.begin();it!=hxyplane.end();it++)
    it->second->Reset();
  npe.clear();
  if (!fUserSetNtracks || fFtrack > mc->GetMCTrackCount()) fFtrack = mc->GetMCTrackCount();
  //Load tracks
  for (int itr = fItrack; itr < fFtrack; itr++) {
    
    if(DEBUG) std::cout<<"  Track "<<itr<<std::endl;
    
    RAT::DS::MCTrack *mctrack = mc->GetMCTrack(itr);
    //Create new track
    pl_tracks.resize(pl_tracks.size()+1);
    //Set PDGcode color code
    CustomizeTrack(&pl_tracks.back(), mctrack);
    //Measure electron length
    if(mctrack->GetPDGCode()==11) elength += mctrack->GetLength();
    //Count processes
    RAT::DS::MCTrackStep *firststep = mctrack->GetMCTrackStep(0);
    RAT::DS::MCTrackStep *laststep = mctrack->GetLastMCTrackStep();
    double last_pos[3];
    laststep->GetEndpoint().GetXYZ(last_pos);
    //      if (itr>=10 && itr<11) std::cout<<" Last pos: "<<last_pos[0]<<" "<<last_pos[1]<<" "<<last_pos[2]<<" "<<std::endl;
    FirstProcessCounter[firststep->GetProcess()] += 1;
    LastProcessCounter[laststep->GetProcess()] += 1;

    //Make XY-plane for ring display
    //Loop over all the steps
    int nsteps = mctrack->GetMCTrackStepCount();
    TVector3 top_pos(-9999.,-9999.,-9999.); //first interpolation point
    TVector3 bottom_pos(9999.,9999.,9999.); //second interpolation point
    TVector3 int_pos(9999.,9999.,9999.);//intersection point
    for (int istep = 0; istep < nsteps; istep++) {
      
      if(DEBUG) std::cout<<"  |->Step "<<istep<<std::endl;

      RAT::DS::MCTrackStep *step = mctrack->GetMCTrackStep(istep);
      const TVector3 endpointstep = step->GetEndpoint();
      pl_tracks.back().SetPoint(istep,endpointstep.X(),endpointstep.Y(),endpointstep.Z());

      //Calculate intersection with XY plane
      // std::cout<<"step "<<istep<<" "<<top_pos.X()<<" "<<top_pos.Y()<<" "<<top_pos.Z()<<std::endl;
      // std::cout<<"step "<<istep<<" "<<bottom_pos.X()<<" "<<bottom_pos.Y()<<" "<<bottom_pos.Z()<<std::endl;
      if(mctrack->GetPDGCode()!=0 && mctrack->GetPDGCode()!=9999) continue; //only for OPTICAL photons

      if(bottom_pos.Z()!=-9999.){ //we haven't found the point yet
      	if(endpointstep.Z()>XP_ZPOS){
	  if(DEBUG) std::cout<<"      Case 1: "<<std::endl;
	  top_pos = endpointstep;
	}
      	else if(top_pos.Z()!=-9999.){ //this is our guy

	  if(DEBUG) std::cout<<"      Case 2: "<<firststep->GetProcess()<<std::endl;

      	  bottom_pos = endpointstep;
      	  //Intersect!
      	  double lambda = (XP_ZPOS - top_pos.Z())/(bottom_pos.Z() - top_pos.Z());
      	  int_pos = top_pos + (bottom_pos - top_pos)*lambda;
      	  //	  std::cout<<"FILL IT! "<<int_pos.X()<<" "<<int_pos.Y()<<" "<<int_pos.Z()<<std::endl;
      	  hxyplane[firststep->GetProcess()]->Fill(int_pos.X(),int_pos.Y());
	  bottom_pos.SetZ(-9999.);

	}
      }

      if(DEBUG) std::cout<<"      (Passed intersection) "<<std::endl;
      
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
  PMTWaveforms.resize(mc->GetMCPMTCount());
  PMTDigitizedWaveforms.resize(mc->GetMCPMTCount());
  double ymin=9999999.; //yaxis min limit analogue
  int ymax_d=0.; //yaxis max limit digital
  int ymin_d=9999999.; //yaxis min limit digital
  double ymax_temp=0.;
  double ymin_temp=0.;
  double xmax_temp=0.;//dummy
  double xmin_temp=0.;//dummy

  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
    
    RAT::DS::MCPMT *mcpmt = mc->GetMCPMT(ipmt);

    //Set analogue graphs
    vPMTWaveforms[ipmt] = mcpmt->GetWaveform();
    for(int isample=0; isample<vPMTWaveforms[ipmt].size(); isample++){
      //      std::cout<<"waveform "<<isample<<" "<<vPMTWaveforms[ipmt][isample]<<std::endl;
      PMTWaveforms[ipmt].SetPoint(isample,isample,vPMTWaveforms[ipmt][isample]);
      ymin = TMath::Min(ymin,vPMTWaveforms[ipmt][isample]);
    }

    //Set digitized graphs
    vPMTDigitizedWaveforms[ipmt] = mcpmt->GetDigitizedWaveform();
    for(int isample=0; isample<vPMTDigitizedWaveforms[ipmt].size(); isample++){
      PMTDigitizedWaveforms[ipmt].SetPoint(isample,isample,vPMTDigitizedWaveforms[ipmt][isample]);
      ymax_d = TMath::Max(ymax_d,vPMTDigitizedWaveforms[ipmt][isample]);
      ymin_d = TMath::Min(ymin_d,vPMTDigitizedWaveforms[ipmt][isample]);
    }

  }
 
  //Set correct limits for drawing purposes
  for (int ipmt = 0; ipmt < mc->GetMCPMTCount(); ipmt++) {
    PMTWaveforms[ipmt].GetYaxis()->SetRangeUser(1.2*ymin,.5);
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
    for (int itr = 0; itr < fFtrack-fItrack; itr++) {
      pl_tracks[itr].Draw("LINE same");
    }
    
  }
  
  if(DRAWWAVEFORMS){
    if(mc->GetMCPMTCount()>0){
    
      if(DEBUG) std::cout<<"Display canvas 2 "<<std::endl;
      
      canvas_event->cd(3);
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
    hxyplane["start"]->SetLineColor(ParticleColor[0]);
    hxyplane["Cerenkov"]->SetLineColor(ParticleColor[0]);
    hxyplane["Scintillation"]->SetLineColor(ParticleColor[9999]);
    if(hxyplane["Cerenkov"]->GetEntries()>0)
      hxyplane["Cerenkov"]->Draw("box");
    else if(hxyplane["Scintillation"]->GetEntries()>0)
      hxyplane["Scintillation"]->Draw("box");
    else
      hxyplane["start"]->Draw("box");
    for (std::map<std::string,TH2F*>::iterator it=hxyplane.begin();it!=hxyplane.end();it++){
      it->second->Draw("box same");
    }
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

void EventDisplay::DumpDisplayInfo(){

  std::cout<<"***NAVIGATION CONTROL***"<<std::endl;
  std::cout<<"Left: H"<<std::endl;
  std::cout<<"Right: L"<<std::endl;
  std::cout<<"Up: U"<<std::endl;
  std::cout<<"Down: I"<<std::endl;
  std::cout<<"Zoom in: J"<<std::endl;
  std::cout<<"Zoom out: K"<<std::endl;
  
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
      if(std::string(fOpt) == "cherenkov" && !ed->IsCerenkov()) continue;
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
    ed->DumpDisplayInfo();
    if(DEBUG) std::cout<<" After Dump Event "<<std::endl;
    ed->DisplayEvent(fEvent);
    if(DEBUG) std::cout<<" After Display Event "<<std::endl;
  }
  
  dummy_app.Run();
  return 0;

  
}


void ParseArgs(int argc, char **argv){

  //Check args
  if(argc<2 || std::string(argv[1])=="-help"){
    std::cerr<<" Usage: EventDisplay.exe INPUTFILE [(optional) -e EVNUMBER -o OPTION -ti INITIAL_TRACK_TO_BE_DRAWN -tf FINAL_TRACK_TO_BE_DRAWN]"<<std::endl;
    exit(0);
  }

  for(int i = 1; i < argc; i++){
    fInputFile = argv[1];
    if(std::string(argv[i]) == "-e") {fEvent = std::stoi(argv[++i]);}
    if(std::string(argv[i]) == "-o") {fOpt = argv[++i];}
    if(std::string(argv[i]) == "-ti") {fItrack = std::stoi(argv[++i]);}
    if(std::string(argv[i]) == "-tf") {fFtrack = std::stoi(argv[++i]); fUserSetNtracks=true;}
  }
  if(DEBUG){
    std::cout<<" Input file "<<fInputFile<<std::endl;
    std::cout<<" Event "<<fEvent<<std::endl;
    std::cout<<" Option "<<fOpt<<std::endl;
  }

    
}
