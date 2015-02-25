#include <vector>
#include <RAT/DAQProc.hh>
#include <RAT/DB.hh>
#include <G4ThreeVector.hh>
#include <RAT/DetectorConstruction.hh>
#include <RAT/PMTPulse.hh>
#include <RAT/DS/PMTWaveform.hh>
#include <CLHEP/Random/RandGauss.h>

using namespace std;

namespace RAT {
  
  inline bool Cmp_PMTPulse_TimeAscending(const PMTPulse *a,
					 const PMTPulse *b)
  {
    double atime = a->GetPulseStartTime();
    double btime = b->GetPulseStartTime();
    return atime < btime;
  }
  
  DAQProc::DAQProc() : Processor("daq") {
    fLdaq = DB::Get()->GetLink("DAQ");

    //sampling time in ns --- this is the size of a PMT time window
    fSamplingTimeDB = fLdaq->GetD("sampling_time");
    //integration time in ns
    fIntTimeDB = fLdaq->GetD("int_time");
    //width of a PMT pulse in ns
    fPulseWidthDB = fLdaq->GetD("pulse_width");
    //offset of a PMT pulse in mV
    fPulseOffsetDB = fLdaq->GetD("pulse_offset");
    //stepping time for discrimination
    fStepTimeDB = fLdaq->GetD("step_time");
    //Minimum pulse height to consider
    fPulseMinDB = fLdaq->GetD("pulse_min");
    //time before discriminator fires that sampling gate opens
    fGDelayDB = fLdaq->GetD("gate_delay");
    //width of noise in adc counts
    fNoiseAmplDB = fLdaq->GetD("noise_amplitude");
    //PMT thresholds
    fTriggerThresholdDB = fLdaq->GetD("trigger_threshold");
    //Pulse type: 0=square pulses, 1=real pulses
    fPulseTypeDB = fLdaq->GetI("pulse_type");
    //mean of a PMT pulse in ns
    fPulseMeanDB = fLdaq->GetD("pulse_mean");
 
    
    detail << "DAQProc: DAQ constants loaded" << newline;
    detail << "  PMT Pulse type: " << (fPulseTypeDB==0 ? "square" : "realistic") << newline;
    detail << dformat("  PMT Pulse Width: ....................... %5.1f ns\n", fPulseWidthDB);
    detail << dformat("  PMT Pulse Offset: ...................... %5.1f ADC Counts\n", fPulseOffsetDB);
    detail << dformat("  Min PMT Pulse Height: .................. %5.1f mV\n", fPulseMinDB);
    detail << dformat("  PMT Channel Integration Time: .... %6.2f ns\n", fIntTimeDB);
    detail << dformat("  PMT Channel Total Sample Time: ......... %6.2f ns\n", fSamplingTimeDB);
    detail << dformat("  PMT Channel Stepping Time: ............. %6.2f ns\n", fStepTimeDB);
    detail << dformat("  Channel Gate Delay: .................... %5.1f ns\n", fGDelayDB);
    detail << dformat("  Hi Freq. Channel Noise: ................ %6.2f adc counts\n", fNoiseAmplDB);
    detail << dformat("  PMT Trigger threshold: ......................... %5.1f mV\n", fTriggerThresholdDB);
    detail << dformat("  PMT Pulse Mean: ........................ %5.1f\n", fPulseMeanDB);

    fEventCounter = 0;
  }
  
  Processor::Result DAQProc::DSEvent(DS::Root *ds) {
    //This processor build waveforms for each PMT in the MC generated event, sample them and
    //store each sampled piece as a MCPMTSampled.

    DS::MC *mc = ds->GetMC();
    if(ds->ExistEV()) {  // there is already a EV branch present 
      ds->PruneEV();     // remove it, otherwise we'll have multiple detector events
                         // in this physics event
                         // we really should warn the user what is taking place
    }


    //Loop through the PMTs in the MC generated event
    for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
      
      DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
      
      //For each PMT loop over hit photons and create a waveform for each of them
      DS::PMTWaveform pmtwf;
      pmtwf.SetStepTime(fStepTimeDB);
      double TimePhoton;
      double PulseDuty=0.0;

      for (size_t iph=0; iph < mcpmt->GetMCPhotonCount(); iph++) {
	
	DS::MCPhoton *mcphotoelectron = mcpmt->GetMCPhoton(iph);
	//      TimePhoton = mcphotoelectron.GetFrontEndTime();
	TimePhoton = mcphotoelectron->GetHitTime(); //fixme: not sure about this time...
	
	//Produce pulses and add them to the waveform
	PMTPulse *pmtpulse;
	if (fPulseTypeDB==0){
            pmtpulse = new SquarePMTPulse; //square PMT pulses
	}
	else{
	  pmtpulse = new RealPMTPulse; //real PMT pulses shape
	}
	
	pmtpulse->SetPulseMean(fPulseMeanDB);
	pmtpulse->SetStepTime(fStepTimeDB);
	pmtpulse->SetPulseMin(fPulseMinDB);
	pmtpulse->SetPulseCharge(mcphotoelectron->GetCharge());
	pmtpulse->SetPulseWidth(fPulseWidthDB);
	pmtpulse->SetPulseOffset(fPulseOffsetDB);
	pmtpulse->SetPulseStartTime(TimePhoton); //also sets end time according to the pulse width and the pulse mean
	pmtwf.fPulse.push_back(pmtpulse);
	PulseDuty += pmtpulse->GetPulseEndTime() - pmtpulse->GetPulseStartTime();

      } // end mcphotoelectron loop: all pulses produces for this PMT
      
      //Sort pulses in time order
      std::sort(pmtwf.fPulse.begin(),pmtwf.fPulse.end(),Cmp_PMTPulse_TimeAscending);

      //At this point the PMT waveform is defined for the whole event for this PMT so save it
      mcpmt->SetWaveform(pmtwf);

      //Sample the PMT waveform to look for photoelectron hits and create a new MCPMTSample
      //for every one of them, regarless they cross threshold. A flag is raised if the threshold
      //is crossed
      while (pmtwf.fPulse.size()>0){
	
	double TimeNow = pmtwf.fPulse[0]->GetPulseStartTime();
	double LastPulseTime = pmtwf.fPulse[pmtwf.fPulse.size()-1]->GetPulseEndTime();
	int NextPulse=0;
	double wfheight = 0.0;
	float NoiseAmpl = fNoiseAmplDB/sqrt(PulseDuty/fStepTimeDB);
	float qfuzz=0.0;

	//Check if the waveform crosses the threshold
	while( qfuzz < fTriggerThresholdDB && TimeNow < LastPulseTime){
	  wfheight =  pmtwf.GetHeight(TimeNow); //height of the pulse at this step
          qfuzz = wfheight+NoiseAmpl*CLHEP::RandGauss::shoot();
	  
	  // move forward in time by step or if we're at baseline move ahead to next pulse
	  if (wfheight==0.0){
	    NextPulse = pmtwf.GetNext(TimeNow);
	    if (NextPulse>0)
	      {TimeNow = pmtwf.fPulse[NextPulse]->GetPulseStartTime();}
	    else
	      {TimeNow= LastPulseTime+1.;}
	  }
	  else{
	    TimeNow += fStepTimeDB;
	  }
	}
	
	//If this PMT crosses the threshold set the hit time as the time when it crosses the
	//threshold and set the flag to true. If doesn't, set the hit time to the starting time
	//of the pulse.
	double HitStartTime = pmtwf.fPulse[0]->GetPulseStartTime();
	bool IsAboveThreshold = false;
	if (TimeNow < LastPulseTime){ //means that we have crossed threshold in the time window
	  IsAboveThreshold = true;
	  HitStartTime = TimeNow;
	}
	
	//Integrate charge and create a new PMT in the event
	double IntegratedCharge = 0.;
	double TimeStartSample = HitStartTime - fGDelayDB; //start before several steps before thresholds
	double TimeEndSample = TimeStartSample+fSamplingTimeDB;
	double TimeEndIntegration = TimeStartSample+fIntTimeDB;
	
	unsigned int ipulse=0;
	while (ipulse < pmtwf.fPulse.size() && pmtwf.fPulse[ipulse]->GetPulseStartTime()<TimeEndSample){
	  IntegratedCharge+=pmtwf.fPulse[ipulse]->Integrate(TimeStartSample,TimeEndIntegration); //Fixme: create a function in PMTWaveForm that performs the integration
	  ipulse++;
	}

	//Go until the last pulse in the sampling window to start over again
	while (ipulse < pmtwf.fPulse.size() && pmtwf.fPulse[ipulse]->GetPulseStartTime()<TimeEndSample ) {
	  ipulse++;
	}
	
	//Set PMT observables and save it as a new PMT object in the event
	DS::PMT* mcpmtsample = mc->AddNewMCPMTSampled();
	mcpmtsample->SetID(mcpmt->GetID());
	mcpmtsample->SetCharge(IntegratedCharge);
	mcpmtsample->SetTime(HitStartTime);
	mcpmtsample->SetAboveThreshold(IsAboveThreshold);

	//Remove all the pulses whithin the sampling window
	pmtwf.fPulse.erase(pmtwf.fPulse.begin(),pmtwf.fPulse.begin()+ipulse);
	
      } //end pulse sampling
      
      //clean up just in case
      for (unsigned int i = 0; i<pmtwf.fPulse.size();i++){
        delete pmtwf.fPulse[i];
      }
      
    } //end PMT loop

    return Processor::OK;

  } //DAQProc::DSEvent
  
} // namespace RAT
