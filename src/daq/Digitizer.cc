#include <iostream>
#include <RAT/Digitizer.hh>
#include <CLHEP/Random/RandGauss.h>

namespace RAT {

  Digitizer::Digitizer(){}
  Digitizer::~Digitizer(){}
  
  
  void Digitizer::GenerateElectronicNoise(int ichannel, DS::PMTWaveform pmtwf) {
    
    //*** deprecated
    //Sort pulses in time order
    //    std::sort(fPulse.begin(),fPulse.end(),Cmp_PMTPulse_TimeAscending);
    // double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
    // double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
    // if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
    //    int nsamples = (endtime - starttime)/fStepTime;
    // double PulseDuty=0.0;
    // for(int ipulse = 0; ipulse<pmtwf.fPulse.size(); ipulse++)
    //   PulseDuty +=  pmtwf.fPulse[ipulse]->GetPulseEndTime() - pmtwf.fPulse[ipulse]->GetPulseStartTime();
    // float NoiseAmpl = fNoiseAmpl/sqrt(PulseDuty/fStepTime);
    //    std::cout<<starttime<<" "<<endtime<<" "<<nsamples<<" "<<PulseDuty<<" "<<NoiseAmpl<<std::endl;
    //*********
    
    //Sort pulses in time order
    //    std::sort(fPulse.begin(),fPulse.end(),Cmp_PMTPulse_TimeAscending);
    double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
    double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
    if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
    int nsamples = (endtime - starttime)/fStepTime;
    fNoise[ichannel].resize(nsamples);
    float NoiseAmpl = fNoiseAmpl;
    for(int istep=0; istep<nsamples ;istep++){
      fNoise[ichannel][istep] = NoiseAmpl*CLHEP::RandGauss::shoot();
    }
    
  }

  //*** deprecated
  // void Digitizer::DigitizeWaveForm(DS::PMTWaveform pmtwf){
    
  //   GenerateElectronicNoise(pmtwf); //Fill fNoise vector

  //   double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
  //   double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
  //   if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
  //   int nsamples = (endtime - starttime)/fStepTime;
  //   int nADCs = 1 << fNBits; //Calculate the number of adc counts
  //   double adcpervolt = nADCs/(fVhigh - fVlow);
  //   double charge = 0.;

  //   //    std::cout<<starttime<<" "<<endtime<<" "<<nsamples<<" "<<nADCs<<" "<<adcpervolt<<std::endl;
    
  //   double currenttime = starttime;
  //   double volt = 0.;
  //   int adcs = 0.;
  //   for(int isample = 0; isample<nsamples; isample++){
  //     charge = pmtwf.GetHeight(currenttime)*fStepTime;
  //     volt = charge*fResistance/fStepTime; //convert to voltage
  //     volt = volt + fNoise[isample]; //add electronic noise
  //     adcs = round((volt - fVlow + fOffset)*adcpervolt); //digitize: V->ADC
  //     //      charge += (pmtwf.GetHeight(currenttime)+fNoise[isample])*fStepTime; //not used, just for sanity check

  //     //Manage voltage saturation
  //     if(adcs<0) adcs = 0;
  //     else if(adcs>=nADCs) adcs = nADCs - 1;
      
  //     //Save sample
  //     fDigitWaveForm.push_back(adcs);
  //     //      std::cout<<isample<<" "<<volt<<" "<<adcs<<" "<<pmtwf.GetHeight(currenttime)<<" "<<fNoise[isample]<<std::endl;

  //     //Step on time
  //     currenttime+=fStepTime;
  //   }

  //   //    std::cout<<"Analogue integrated charge "<<charge<<std::endl;
    
    
  // }
  //**********




  void Digitizer::AddChannel(int ichannel, DS::PMTWaveform pmtwf){
    
    GenerateElectronicNoise(ichannel,pmtwf); //Fill fNoise vector

    double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
    double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
    if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
    int nsamples = (endtime - starttime)/fStepTime;
    int nADCs = 1 << fNBits; //Calculate the number of adc counts
    double adcpervolt = nADCs/(fVhigh - fVlow);
    double charge = 0.;

    //    std::cout<<starttime<<" "<<endtime<<" "<<nsamples<<" "<<nADCs<<" "<<adcpervolt<<std::endl;
    
    double currenttime = starttime;
    double volt = 0.;
    int adcs = 0.;
    for(int isample = 0; isample<nsamples; isample++){
      charge = pmtwf.GetHeight(currenttime)*fStepTime;
      volt = charge*fResistance/fStepTime; //convert to voltage
      volt = volt + fNoise[ichannel][isample]; //add electronic noise
      adcs = round((volt - fVlow + fOffset)*adcpervolt); //digitize: V->ADC
      //      charge += (pmtwf.GetHeight(currenttime)+fNoise[isample])*fStepTime; //not used, just for sanity check

      //Manage voltage saturation
      if(adcs<0) adcs = 0;
      else if(adcs>=nADCs) adcs = nADCs - 1;
      
      //Save sample
      fDigitWaveForm[ichannel].push_back(adcs);
      //      std::cout<<isample<<" "<<volt<<" "<<adcs<<" "<<pmtwf.GetHeight(currenttime)<<" "<<fNoise[ichannel][isample]<<std::endl;

      //Step on time
      currenttime+=fStepTime;
    }

    //    std::cout<<"Analogue integrated charge "<<charge<<std::endl;
    
    
  }


  //This function retrieve a chunk of the digitized waveform in a sampling
  //window defined by [thres_sample-fSampleDelay, thres_sample+fSamplingWindow]
  std::vector<int> Digitizer::SampleWaveform(std::vector<int> completewaveform, int thres_sample){

    int start_sample = thres_sample-fSampleDelay;
    int end_sample = thres_sample+(int)fSamplingWindow/fStepTime;
    std::vector<int> sampledwaveform;
    
    while(start_sample<=end_sample){
      sampledwaveform.push_back(completewaveform[start_sample]);
      start_sample++;
    }

    //    *thres_sample = end_sample;  //set the step at the end of the sampling window
    return sampledwaveform;

  }

  //Moves the sampling point towards the end of the sampling window defined by the
  //user
  int Digitizer::GoToEndOfSample(int isample){

    int end_sample = isample+(int)fSamplingWindow/fStepTime;
    return end_sample;  //set the step at the end of the sampling window

  }

  //Parse threshold in volts and store it in ADC counts  
  void Digitizer::SetThreshold(double threhold_volts){

    int nADCs = 1 << fNBits; //Calculate the number of adc counts
    double adcpervolt = nADCs/(fVhigh - fVlow);
    fDigitizedThreshold = round((threhold_volts - fVlow + fOffset)*adcpervolt); //digitize: V->ADC
    
  }
  
  double Digitizer::IntegrateCharge(std::vector<int> digitizedwaveform){

    int nADCs = 1 << fNBits; //Calculate the number of adc counts
    double voltsperadc = (fVhigh - fVlow)/(double)nADCs;
    double charge = 0.;
    int start_sample = 0;
    while(start_sample<digitizedwaveform.size()){
      charge += ((double)digitizedwaveform[start_sample]*voltsperadc + fVlow)*fStepTime/fResistance; //ADC to charge
      start_sample++;
    }
    //    std::cout<<" Digitized integrated charge "<<charge<<std::endl;

    charge=std::abs(charge); //pulses are negative so covert to positive charge
    return charge;
  }

  void Digitizer::Clear(){

    for(std::map<int, std::vector<int> >::iterator it = fDigitWaveForm.begin(); it!=fDigitWaveForm.end(); it++){
      it->second.clear();
    }
    for(std::map<int, std::vector<double> >::iterator it = fNoise.begin(); it!=fNoise.end(); it++){
      it->second.clear();
    }

    //Nicer but not compatible with C++<11
    // for(auto &it : fDigitWaveForm){
    //   it.second.clear();
    // }
    // for(auto &it : fNoise){
    //   it.second.clear();
    // }

  }
}
