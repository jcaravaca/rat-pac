////////////////////////////////////////////////////////////////////
/// \class RAT::PMTWaveform
///
/// \brief   Sum of PMT pulses
///
/// \author Josh Klein <jrk@hep.upenn.edu>
///
/// REVISION HISTORY:\n
///     10 Sep 2010 : Gabriel Orebi Gann - Bug fix in GetNext routine (it was
///     returning the current pulse, not the next one)
///
/// \details
////////////////////////////////////////////////////////////////////
#ifndef __RAT_PMTWaveform__
#define __RAT_PMTWaveform__

#include <vector>
#include <RAT/PMTPulse.hh>
//#include <TObject.h> //Dummy root include file needed to define the ClassDef and ClassImp methods
#include <TGraph.h> //Dummy root include file needed to define the ClassDef and ClassImp methods

namespace RAT {
namespace DS {

  //class PMTWaveform : public TObject {
class PMTWaveform {
public:

  //  PMTWaveform() : TObject(){}
  PMTWaveform(){}
  virtual ~PMTWaveform() {}
  //  virtual void GenerateElectronicNoise(double);
  virtual float GetHeight(double time);
  virtual int GetNext(double time);
  virtual void SetGraph(); //Set the graph with the pulses that are stored in the moment you call this method
  virtual void SetStepTime(double step){fStepTime=step;};
  virtual TGraph GetGraph(){return gwaveform;};

  std::vector<PMTPulse*> fPulse;

  ClassDef(PMTWaveform, 1)

protected:

  TGraph gwaveform;
  double fStepTime;
  double fEventTime;
  std::vector<double> fNoise;
  
};

}
} // namespace RAT

#endif
