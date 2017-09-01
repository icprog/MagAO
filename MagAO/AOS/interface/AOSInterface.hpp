/* Interface/AOSInterface.h
 *
 * AOSInterface class header file for the AOS subsystem.
 *
 */

#ifndef AOSINTERFACE_H
#define AOSINTERFACE_H

#include <GshmVar.hpp>
#include <iif/tcs/Position.hpp>

#include "aos/aosupervisor/aoslib/aoscodes.h"
#include "aos/aosupervisor/arblib/aoArb/AOArbConst.h"

namespace lbto {

////////////////////////////////////////////////////////////////////////
//  interface class that the AOCommands is derived from.

template <class T>class AOSInterface
{
	//  constructor/destructor
   public:
     virtual ~AOSInterface() {};

//  methods - these should be implemented in the derived class
   public:
      virtual  T   PresetFlat(string) = 0;
      virtual  T   PresetAO(string AOmode, Position refSTar) = 0;
      virtual  T   PresetAOg(string AOmode, string WFS, double romm_x, double romm_y, double mag, double cindex) = 0;
      virtual  T   CheckRefAO() = 0;
      virtual  T   AcquireRefAO(bool withStages) = 0;
      virtual  T   RefineAO(string) = 0;
      virtual  T   ModifyAO(int nModes, double freq, int nBin, 
                            double ttMod, string f1, string f2) = 0;
      virtual  T   StartAO() = 0;
      virtual  T   OffsetXY() = 0;
      virtual  T   OffsetXYg(double x, double y) = 0;
      virtual  T   OffsetZ(double z) = 0;
      virtual  T   CorrectModes(double modes[N_CORRECTMODES]) = 0;
      virtual  T   SetZernikes(int n_zern, double zernikes[N_ZERN]) = 0;
      virtual  T   Stop(string) = 0;
      virtual  T   Pause() = 0;
      virtual  T   Resume() = 0;
      virtual  T   UserPanic(string) = 0;
      virtual  T   SetNewInstrument(string,string) = 0;
      virtual  T   AdsecOn() = 0;
      virtual  T   AdsecOff() = 0;
      virtual  T   AdsecSet() = 0;
      virtual  T   AdsecRest() = 0;
      virtual  T   WfsOn(string wfsid) = 0;
      virtual  T   WfsOff(string wfsid) = 0;
};

}  // namespace lbto 

#endif  //  AOSINTERFACE_H
