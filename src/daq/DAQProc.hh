#ifndef __RAT_DAQProc__
#define __RAT_DAQProc__

#include <RAT/Processor.hh>

namespace RAT {


class DAQProc : public Processor {
public:
  DAQProc();
  virtual ~DAQProc() { };
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int fEventCounter;
  std::vector<double> fSPECharge;
};


} // namespace RAT

#endif
