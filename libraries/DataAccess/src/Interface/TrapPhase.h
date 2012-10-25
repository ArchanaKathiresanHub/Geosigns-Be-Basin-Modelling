#ifndef _INTERFACE_TRAPPHASE_H_
#define _INTERFACE_TRAPPHASE_H_

#include "Interface/DAObject.h"

namespace DataAccess
{
   namespace Interface
   {
      class Reservoir;
      class Snapshot;

      /// TrapPhase Class
      /// This class is used by the LeadTrap class to describe phase properties
      class TrapPhase : public DAObject
      {
	 public:
	    TrapPhase (ProjectHandle * projectHandle, database::Record * record);
	    ~TrapPhase (void);

	    /// return the TrapPhase's porosity
	    virtual double getPorosity (void) const;
	    /// return the TrapPhase's seal permeability
	    virtual double getSealPermeability (void) const;
	    /// return the TrapPhase's gas wetness
	    virtual double getGasWetness (void) const;
	    /// return the TrapPhase's nitrogen to gas ratio
	    virtual double getN2GasRatio (void) const;
	    /// return the TrapPhase's API Gravity
	    virtual double getAPIGravity (void) const;
	    /// return the TrapPhase's condensate gas ratio
	    virtual double getCGR (void) const;
	    /// return the TrapPhase's formation volume factor
	    virtual double getFormVolFactor (void) const;

	    /// return the TrapPhase's gas mass
	    virtual double getGasMass (void) const;
	    /// return the TrapPhase's oil mass
	    virtual double getOilMass (void) const;
	    /// return the TrapPhase's gas density
	    virtual double getGasDensity (void) const;
	    /// return the TrapPhase's oil density
	    virtual double getOilDensity (void) const;
	    /// return the TrapPhase's gas viscosity
	    virtual double getGasViscosity (void) const;
	    /// return the TrapPhase's oil viscosity
	    virtual double getOilViscosity (void) const;
	    /// return the TrapPhase's gas volume
	    virtual double getGasVolume (void) const;
	    /// return the TrapPhase's oil volume
	    virtual double getOilVolume (void) const;

	    virtual void asString (string &) const;

	 private:
      };
   }
}

#endif // _INTERFACE_TRAPPHASE_H_
