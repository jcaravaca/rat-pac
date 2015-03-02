#include <iostream>
#include <RAT/Digitizer.hh>
#include <CLHEP/Random/RandGauss.h>

namespace RAT {


  Digitizer::Digitizer(){}
  Digitizer::~Digitizer(){}
  
  
  void Digitizer::GenerateElectronicNoise(DS::PMTWaveform pmtwf) {
    
    //Sort pulses in time order
    //    std::sort(fPulse.begin(),fPulse.end(),Cmp_PMTPulse_TimeAscending);
    double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
    double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
    if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
    int nsamples = (endtime - starttime)/fStepTime;

    double PulseDuty=0.0;
    for(int ipulse = 0; ipulse<pmtwf.fPulse.size(); ipulse++)
      PulseDuty +=  pmtwf.fPulse[ipulse]->GetPulseEndTime() - pmtwf.fPulse[ipulse]->GetPulseStartTime();

    float NoiseAmpl = fNoiseAmpl/sqrt(PulseDuty/fStepTime);
    //    std::cout<<starttime<<" "<<endtime<<" "<<nsamples<<" "<<PulseDuty<<" "<<NoiseAmpl<<std::endl;

    //  int nsamples = (int)fEventTime/fStepTime;
    fNoise.resize(nsamples);
    for(int istep=0; istep<nsamples ;istep++){
      fNoise[istep] = NoiseAmpl*CLHEP::RandGauss::shoot();
    }
    
  }

  void Digitizer::DigitizeWaveForm(DS::PMTWaveform pmtwf){
    
    GenerateElectronicNoise(pmtwf); //Fill fNoise vector

    double starttime = pmtwf.fPulse.front()->GetPulseStartTime();
    double endtime = pmtwf.fPulse.back()->GetPulseEndTime();
    if(starttime-endtime < fSamplingWindow) endtime = starttime + fSamplingWindow;
    int nsamples = (endtime - starttime)/fStepTime;
    int nADCs = 1 << fNBits; //Calculate the number of adc counts
    int adcpervolt = nADCs/(fVhigh - fVlow);
    double charge = 0.;

    //    std::cout<<starttime<<" "<<endtime<<" "<<nsamples<<" "<<nADCs<<" "<<std::endl;
    
    double currenttime = starttime;
    double volt = 0.;
    int digitvolt = 0.;
    for(int isample = 0; isample<nsamples; isample++){
      volt = pmtwf.GetHeight(currenttime)+fNoise[isample]; //add electronic noise
      volt = volt*fResistance/fStepTime*1e-3; //convert to voltage
      digitvolt = round((volt + fOffset)*adcpervolt); //digitize: V->ADC
      charge += pmtwf.GetHeight(currenttime)+fNoise[isample];

      //Manage voltage saturation
      if(digitvolt<0) digitvolt = 0;
      else if(digitvolt>=nADCs) digitvolt = nADCs - 1;
      
      //Save sample
      fDigitWaveForm.push_back(digitvolt);
      //      std::cout<<volt<<" "<<digitvolt<<" "<<pmtwf.GetHeight(currenttime)<<" "<<fNoise[isample]<<std::endl;

      //Step on time
      currenttime+=fStepTime;
    }

    //    std::cout<<"PMT integrated charge "<<charge<<std::endl;
    
    
  }
  

  //This function retrieve a chunk of the digitized waveform in a sampling
  //window defined by [thres_sample-fSampleDelay, thres_sample+fSamplingWindow]
  std::vector<int> Digitizer::SampleWaveform(std::vector<int> completewaveform, int *thres_sample){

    int start_sample = *thres_sample-fSampleDelay;
    int end_sample = *thres_sample+(int)fSamplingWindow/fStepTime;
    std::vector<int> sampledwaveform;
    
    while(start_sample<=end_sample){
      sampledwaveform.push_back(completewaveform[start_sample]);
      start_sample++;
    }

    *thres_sample = end_sample;  //set the step at the end of the sampling window
    return sampledwaveform;

  }

  double Digitizer::GetChargeForSample(std::vector<int> completewaveform){

    int nADCs = 1 << fNBits; //Calculate the number of adc counts
    double voltsperadc = (fVhigh - fVlow)/(double)nADCs;
    int charge = 0;
    int start_sample = 0;
    while(start_sample<completewaveform.size()){
      charge += completewaveform[start_sample];
      start_sample++;
    }
    charge *= voltsperadc*fStepTime/fResistance*1e3; //volts to pC
    //    std::cout<<" charge "<<charge<<std::endl;

    return charge;
  }
  
}
