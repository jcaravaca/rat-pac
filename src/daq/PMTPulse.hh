////////////////////////////////////////////////////////////////////
#ifndef __RAT_PMTPulse__
#define __RAT_PMTPulse__

#include <vector>

namespace RAT {

class PMTPulse {
public:

  PMTPulse();
  virtual ~PMTPulse();

//  virtual void SetPulseStartTime(double time){fStartTime = time;};
  virtual void SetPulseWidth(float width){fPulseWidth=width;};
  virtual void SetPulseCharge(float charge)=0;
  virtual void SetPulseMean(float mean){fPulseMean = mean;};
  virtual void SetPulseOffset(float offset){fPulseOffset=offset;};
  virtual void SetPulseStartTime(double time)=0;
  virtual void SetStepTime(double step){fStepTime=step;};
  virtual void SetPulseMin(float min){fPulseMin=min;};

//  virtual double GetPulseStartTime()=0;
  virtual double GetPulseStartTime() const {return fStartTime;};
  virtual float GetPulseWidth(){return fPulseWidth;};
  virtual float GetPulseCharge()=0;
  virtual float GetPulseHeight(double time)=0;
  virtual double GetPulseEndTime()=0;
  virtual float Integrate(double time1, double time2)=0;

protected:

  double fStartTime;
  float fPulseWidth;
  float fPulseMean;
  float fPulseOffset;
  double fStepTime;
  float fPulseMin;

};

class SquarePMTPulse : public PMTPulse {

public:
  SquarePMTPulse(){};
  virtual ~SquarePMTPulse(){};

  virtual void SetPulseCharge(float _fPulseCharge){fPulseCharge = _fPulseCharge;};

  virtual void SetPulseStartTime(double time);
  virtual float GetPulseCharge(){return fPulseCharge;};
  virtual float GetPulseHeight(double time);
  virtual double GetPulseEndTime(){return fEndTime;};
  virtual float Integrate(double time1, double time2);

private:

  float fPulseCharge;
  double fEndTime;

};

class RealPMTPulse : public PMTPulse {

public:
  RealPMTPulse(){};
  virtual ~RealPMTPulse(){};

  virtual void SetPulseCharge(float _fPulseCharge){fPulseCharge = _fPulseCharge;};
  virtual void SetPulseStartTime(double _fStartTime);

  virtual float GetPulseCharge(){return fPulseCharge;};
  virtual float GetPulseHeight(double time);
  virtual double GetPulseEndTime(){return fEndTime;};
  virtual float Integrate(double time1, double time2);

private:

//  double fStartTime;
  float fPulseCharge;
  double fEndTime;

};

} // namespace RAT

#endif
