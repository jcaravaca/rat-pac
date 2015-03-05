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
#include<TLegend.h>
#include<TMath.h>
#include<TStyle.h>
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include<RAT/DS/MC.hh>
#include<RAT/DS/MCTrack.hh>
#include<RAT/DS/MCTrackStep.hh>
#include<RAT/DS/MCPMT.hh>
#include<RAT/DSReader.hh>
#include<RAT/DS/Root.hh>
#include <RAT/DB.hh>

#define NBINS 500


namespace TheiaRnD{


class Fitter{

public:

  Fitter(int, char**);
  ~Fitter(){};
  void ParseArgs(int argc, char **argv);
  void GetPDFs();
  double Likelihood(const double*); //likelihood function
  double ChiSquare(const double*); //chisquare function
  void DoFit();
  std::vector<TH1F*> hpdf_mc; //prefit MC pdfs
  std::vector<TH1F*> hpdf_dt; //prefit DT pdfs
  std::vector<TH1F*> hpdf_mc_fit; //posfit MC pdfs
  std::vector<TH1F*> hpdf_dt_fit; //posfit DT pdfs
  TH1F* hpdf_mc_sum;
  TH1F* hpdf_mc_sum_fit;


protected:

  //Input files
  std::map<int, char*> fMCFiles; //0->90Sr, 1->90Y
  std::map<int, char*> fDATAFiles; //0->Backgrounds, 1-> Cerenkov
  bool exist_srfile = false;
  bool exist_yfile = false;
  bool exist_datafile = false;
  bool exist_bkgfile = false;

  //  TMinuit *fMinuit;

  ROOT::Math::Minimizer* min;
  double fMinLikelihood;
  std::vector<double> fParFitted;
  std::vector<double> fParErrFitted;
  
};

//Constructor
Fitter::Fitter(int argc, char **argv){
  ParseArgs(argc, argv);
  GetPDFs();
}

//Parser for main() arguments
void Fitter::ParseArgs(int argc, char **argv){
  for(int i = 1; i < argc; i++){
    if(std::string(argv[i]) == "-s") {fMCFiles[0] = argv[++i]; exist_srfile=true;} //signal - strontium
    if(std::string(argv[i]) == "-y") {fMCFiles[1] = argv[++i]; exist_yfile=true;} //signal - ytrium
    if(std::string(argv[i]) == "-b") {fDATAFiles[0] = argv[++i]; exist_bkgfile=true;} //data - background
    if(std::string(argv[i]) == "-d") {fDATAFiles[1] = argv[++i]; exist_datafile=true;} //data - cerenkov
  }
  
  if(!exist_srfile || !exist_yfile || !exist_bkgfile || !exist_datafile){
    std::cerr<<" Specify input files with options: '-s', '-y', '-d' or '-b'"<<std::endl;
    std::cerr<<" Fit cannot be perform! Drawing prefit histograms..."<<std::endl;
    exit(0);
  }
}

//Extract PDFs from DATA and MC input files and populate the TH1F vectors
void Fitter::GetPDFs(){

  //Get real data
  TGraph* gpdf_dt;
  TH1F* hdata;
  TH1F* hscale;
  for(int ifile=0; ifile<fDATAFiles.size(); ifile++){
    hdata = new TH1F(Form("hdata%i",ifile),"hdata",NBINS,0,100);
    hscale = new TH1F(Form("hscale%i",ifile),"hscale",NBINS,0,100); //count how many times we fill the bin and scale it, root is anoying...
    gpdf_dt = new TGraph(fDATAFiles[ifile],"%lg %lg",",");
    double x=0.;
    double y=0.;
    for(int ip=0; ip<gpdf_dt->GetN(); ip++){
      gpdf_dt->GetPoint(ip,x,y);
      hdata->Fill(x*1.6,y);
      hscale->Fill(x*1.6,1.);
    }
    hdata->Divide(hscale);
    //    hdata->Scale(1./hdata->Integral()); //Nomalize for shape only analysis
    hpdf_dt.push_back(hdata);
  }
  hpdf_dt[0]->Scale(1.6/4.7);//scale background
  
  //Get MC
  RAT::DSReader *dsreader;
  TH1F *htemp;
  for(int ifile=0; ifile<fMCFiles.size(); ifile++){
    dsreader = new RAT::DSReader(fMCFiles[ifile]);
    htemp = new TH1F(Form("hpdf_mc%i",ifile),"Charge",NBINS,0,100);
    dsreader->GetT()->Draw(Form("ds.ev.pmt.charge>>hpdf_mc%i",ifile));
    hpdf_mc.push_back(htemp);
  }

  //Scale MC accoring to the DATA considering 1to1 rate for 90Sr and 90Y
  //  for(int ih=0; ih<hpdf_mc.size(); ih++)
  hpdf_mc[0]->Scale((hpdf_dt[1]->Integral()-hpdf_dt[0]->Integral())/hpdf_mc[0]->Integral()/2.);
  hpdf_mc[1]->Scale((hpdf_dt[1]->Integral()-hpdf_dt[0]->Integral())/hpdf_mc[1]->Integral()/2.);

  //Sum up noise+90Sr+90Y
  hpdf_mc_sum = (TH1F*)hpdf_mc[0]->Clone();
  hpdf_mc_sum->Reset();
  for(int ih=0; ih<hpdf_mc.size(); ih++)
    hpdf_mc_sum->Add(hpdf_mc[ih]);
  hpdf_mc_sum->Add(hpdf_dt[0]);

  //Initialize post fit histograms
  for(int ih=0; ih<hpdf_mc.size(); ih++)
    hpdf_mc_fit.push_back((TH1F*)hpdf_mc[ih]->Clone());
  for(int ih=0; ih<hpdf_dt.size(); ih++)
    hpdf_dt_fit.push_back((TH1F*)hpdf_dt[ih]->Clone());

}

//Likelihood function
double Fitter::Likelihood(const double *par){

  double p_sr = par[0];
  double p_y = par[1];
  double p_bkg = par[2];
  
  double likelihood=0.;
  double n_mc=0.;
  double n_dt=0.;

  for(int ibin=0; ibin<NBINS; ibin++){
    n_mc = p_sr*hpdf_mc[0]->GetBinContent(ibin) +  p_y*hpdf_mc[1]->GetBinContent(ibin) + p_bkg*hpdf_dt[0]->GetBinContent(ibin);
    n_dt = hpdf_dt[1]->GetBinContent(ibin);
    //    likelihood += n_mc - n_dt + n_dt*( log( TMath::Max(n_dt,0.000001) ) - log( TMath::Max(n_mc,0.000001) ) );
    if(n_mc>0 && n_dt>0)
      likelihood += n_mc - n_dt + n_dt*TMath::Log(n_dt/n_mc);
  }
  
  std::cout<<" Likelihood-> step: "<<likelihood<<" "<<p_sr<<" "<<p_y<<" "<<p_bkg<<std::endl;
  return 2*likelihood;

}

//ChiSquare function
double Fitter::ChiSquare(const double *par){

  double p_sr = par[0];
  double p_y = par[1];
  double p_bkg = par[2];
  
  double chisquare=0.;
  double n_mc=0.;
  double n_dt=0.;

  for(int ibin=0; ibin<NBINS; ibin++){
    n_mc = p_sr*hpdf_mc[0]->GetBinContent(ibin) +  p_y*hpdf_mc[1]->GetBinContent(ibin) + p_bkg*hpdf_dt[0]->GetBinContent(ibin);
    n_dt = hpdf_dt[1]->GetBinContent(ibin);
    if(n_mc>0 && n_dt>0)
      chisquare += pow((n_dt - n_mc),2)/n_mc;
  }
  
  std::cout<<" ChiSquare-> step: "<<chisquare<<" "<<p_sr<<" "<<p_y<<" "<<p_bkg<<std::endl;
  return chisquare;

}


  
//Do fit
void Fitter::DoFit(){

  min = ROOT::Math::Factory::CreateMinimizer("Minuit2","Migrad");

  //Set Function
  ROOT::Math::Functor f(this, &Fitter::Likelihood,3);
  //  ROOT::Math::Functor f(this, &Fitter::ChiSquare,3);
  min->SetFunction(f);
  
  // set tolerance , etc...
  min->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2 
  min->SetMaxIterations(10000);  // for GSL 
  min->SetTolerance(0.1);
  min->SetPrintLevel(1);

  // Set the free variables to be minimized!
  min->SetVariable(0,"90Sr",1.,0.01);
  min->SetVariableLimits(0,0.,2.);
  min->SetVariable(1,"90Y",1.,0.01);
  min->SetVariableLimits(1,0.,2.);
  min->SetVariable(2,"Noise",1.,0.01);
  min->SetVariableLimits(2,0.,2.);

  //  do the minimization
  min->Minimize();

  fMinLikelihood = min->MinValue(); //Minimum likelihood
  const double *xs = min->X();
  for(int ipar=0; ipar<3; ipar++)
    fParFitted.push_back(xs[ipar]);

  //***Prepare best fit histograms
  hpdf_mc_fit[0]->Scale(fParFitted[0]);
  hpdf_mc_fit[1]->Scale(fParFitted[1]);
  hpdf_dt_fit[0]->Scale(fParFitted[2]);

  hpdf_mc_sum_fit = (TH1F*)hpdf_mc_fit[0]->Clone();
  hpdf_mc_sum_fit->Add(hpdf_mc_fit[1]);
  hpdf_mc_sum_fit->Add(hpdf_dt_fit[0]);

}

  
}//end TheiaRnD namespace




int main(int argc, char **argv){

  int appargc = 0;
  char **appargv = NULL;
  TApplication dummy("App", &appargc, appargv);
  gStyle->SetOptStat(0);
  
  TheiaRnD::Fitter myfitter(argc, argv);
  myfitter.DoFit();

  TLegend *leg = new TLegend(0.5,0.5,0.9,0.9);
  leg->AddEntry(myfitter.hpdf_dt[1],"Data","PL");
  leg->AddEntry(myfitter.hpdf_mc_sum,"MC","L");
  leg->AddEntry(myfitter.hpdf_dt[0],"DT-Noise","F");
  leg->AddEntry(myfitter.hpdf_mc[0],"MC-90Sr","L");
  leg->AddEntry(myfitter.hpdf_mc[1],"MC-90Y","L");
  leg->SetFillColor(0);
  leg->SetLineColor(0);
  
  TCanvas *c_fit = new TCanvas("c_fit","c_fit",600,600);
  c_fit->Divide(2,1);
  //Before fit
  c_fit->cd(1);
  c_fit->cd(1)->SetLogy();
  //Data
  myfitter.hpdf_dt[1]->SetLineWidth(2);
  myfitter.hpdf_dt[1]->SetLineColor(1);
  myfitter.hpdf_dt[1]->GetXaxis()->SetTitle("Q(pC)");
  myfitter.hpdf_dt[1]->GetYaxis()->SetRangeUser(1.,1.e6);
  myfitter.hpdf_dt[1]->Draw("");
  //Background
  for(int ih=0; ih<myfitter.hpdf_dt.size()-1; ih++){
    myfitter.hpdf_dt[ih]->SetLineWidth(1);
    myfitter.hpdf_dt[ih]->SetFillColor(kGreen);
    myfitter.hpdf_dt[ih]->SetFillStyle(3001);
    myfitter.hpdf_dt[ih]->SetLineColor(kGreen+1);
    //    myfitter.hpdf_dt[ih]->SetLineStyle(ih+1);
    myfitter.hpdf_dt[ih]->Draw("same");
  }
  //MC
  for(int ih=0; ih<myfitter.hpdf_mc.size(); ih++){
    myfitter.hpdf_mc[ih]->SetLineWidth(1);
    //    myfitter.hpdf_mc[ih]->SetLineStyle(2);
    myfitter.hpdf_mc[ih]->SetLineColor(kOrange+ih);
    myfitter.hpdf_mc[ih]->Draw("same");
  }
  myfitter.hpdf_mc_sum->SetLineWidth(2);
  myfitter.hpdf_mc_sum->SetLineColor(kRed+1);
  myfitter.hpdf_mc_sum->Draw("same");
  //After fit
  c_fit->cd(2);
  c_fit->cd(2)->SetLogy();
  //Data
  myfitter.hpdf_dt[1]->SetLineWidth(2);
  myfitter.hpdf_dt[1]->SetLineColor(1);
  myfitter.hpdf_dt[1]->GetXaxis()->SetTitle("Q(pC)");
  myfitter.hpdf_dt[1]->GetYaxis()->SetRangeUser(1.,1.e6);
  myfitter.hpdf_dt[1]->Draw("");
  for(int ih=0; ih<myfitter.hpdf_dt_fit.size()-1; ih++){
    myfitter.hpdf_dt_fit[ih]->SetLineWidth(1);
    myfitter.hpdf_dt_fit[ih]->SetFillColor(kGreen);
    myfitter.hpdf_dt_fit[ih]->SetFillStyle(3001);
    myfitter.hpdf_dt_fit[ih]->SetLineColor(kGreen);
    //    myfitter.hpdf_dt_fit[ih]->SetLineStyle(ih+1);
    myfitter.hpdf_dt_fit[ih]->Draw("same");
  }
  for(int ih=0; ih<myfitter.hpdf_mc_fit.size(); ih++){
    myfitter.hpdf_mc_fit[ih]->SetLineWidth(1);
    //    myfitter.hpdf_mc_fit[ih]->SetLineStyle(2);
    myfitter.hpdf_mc_fit[ih]->SetLineColor(kOrange+ih);
    myfitter.hpdf_mc_fit[ih]->Draw("same");
  }
  myfitter.hpdf_mc_sum_fit->SetLineWidth(2);
  myfitter.hpdf_mc_sum_fit->SetLineColor(kRed+1);
  myfitter.hpdf_mc_sum_fit->Draw("same");
  leg->Draw("same");
  
  dummy.Run();
  return 0;

}
