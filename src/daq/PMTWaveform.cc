#include <cmath>
#include <algorithm>
#include <vector>
#include <RAT/PMTWaveform.hh>
#include <RAT/PMTPulse.hh>
#include <RAT/Log.hh>
#include <RAT/ListHelp.hh>

namespace RAT {


PMTWaveform::PMTWaveform()
{
}

PMTWaveform::~PMTWaveform()
{
  deepdelete_vector(fPulse);
}

float PMTWaveform::GetHeight(double time)
{
    float height = 0.;
    unsigned int i = 0;
    while (i<fPulse.size() && fPulse[i]->GetPulseStartTime()<=time){
        height+=fPulse[i]->GetPulseHeight(time);
        i++;
    }
    return height;
}

int PMTWaveform::GetNext(double time)
{
    unsigned int i = 0;
    while (i<fPulse.size() && fPulse[i]->GetPulseStartTime()<=time){
        i++;
    }
    if(i==fPulse.size())return 0;
    if(fPulse[i]->GetPulseStartTime()<=time){return 0;}
    else{return i;}
}


} // namespace RAT
