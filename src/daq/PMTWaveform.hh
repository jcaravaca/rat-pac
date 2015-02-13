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

namespace RAT {


class PMTWaveform{
public:

  PMTWaveform();
  virtual ~PMTWaveform();
  virtual float GetHeight(double time);
  virtual int GetNext(double time);

  std::vector<PMTPulse*> fPulse;

};


} // namespace RAT

#endif
