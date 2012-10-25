#ifndef _INTERFACE_LEADTRAP_H_
#define _INTERFACE_LEADTRAP_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A LeadTrap object models a Trap as part of a Lead
      class LeadTrap : public DAObject
      {
	 public:
	    LeadTrap (ProjectHandle * projectHandle, Trap * trap);
	    ~LeadTrap (void);

	    /// Add information of the trap content for a given reservoir phase
	    void createTrapPhase (database::Record * record);

	    /// See if this LeadTrap matches the given conditions
	    bool matchesConditions (const Reservoir * reservoir, unsigned int id);

	    /// Return the id of this LeadTrap
	    virtual unsigned int getId (void) const;

	    /// Return the Snapshot at which this LeadTrap was computed
	    virtual const Snapshot * getSnapshot (void) const;

	    /// Return the Reservoir containing this LeadTrap
	    const Reservoir * getReservoir (void) const;

	    /// Return the real world position of this LeadTrap
	    virtual void getPosition (double & x, double & y) const;

	    /// Return the grid position of this LeadTrap
	    virtual void getGridPosition (unsigned int & i, unsigned int & j) const;

	    /// return the mass of the given component in this LeadTrap
	    virtual double getMass (ComponentId componentId) const;

	    /// return the LeadTrap's depth
	    virtual double getDepth (void) const;
	    /// return the LeadTrap's temperature
	    virtual double getTemperature (void) const;
	    /// return the LeadTrap's pressure
	    virtual double getPressure (void) const;
	    /// return the LeadTrap's gas-oil contact depth
	    virtual double getGOC (void) const;
	    /// return the LeadTrap's oil-water contact depth
	    virtual double getOWC (void) const;
	    /// return the LeadTrap's surface at the oil-water contact depth
	    virtual double getWCSurface (void) const;
            
	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

	    /// return the LeadTrap's porosity
	    virtual double getPorosity (void) const;
	    /// return the LeadTrap's seal permeability
	    virtual double getSealPermeability (void) const;

	    /// return the LeadTrap's gas wetness
	    virtual double getGasWetness (PhaseId phaseId) const;
	    /// return the LeadTrap's nitrogen gas ratio
	    virtual double getN2GasRatio (PhaseId phaseId) const;
	    /// return the LeadTrap's API Gravity
	    virtual double getAPIGravity (PhaseId phaseId) const;
	    /// return the LeadTrap's condensate gas ratio
	    virtual double getCGR (PhaseId phaseId) const;
	    /// return the LeadTrap's formation volume factor
	    virtual double getFormVolFactor (PhaseId phaseId) const;

	    /// return the LeadTrap's gas mass at the specified reservoir phase
	    virtual double getGasMass (PhaseId phaseId) const;
	    /// return the LeadTrap's oil mass at the specified reservoir phase
	    virtual double getOilMass (PhaseId phaseId) const;
	    /// return the LeadTrap's gas density at the specified reservoir phase
	    virtual double getGasDensity (PhaseId phaseId) const;
	    /// return the LeadTrap's oil density at the specified reservoir phase
	    virtual double getOilDensity (PhaseId phaseId) const;
	    /// return the LeadTrap's gas viscosity at the specified reservoir phase
	    virtual double getGasViscosity (PhaseId phaseId) const;
	    /// return the LeadTrap's oil viscosity at the specified reservoir phase
	    virtual double getOilViscosity (PhaseId phaseId) const;
	    /// return the LeadTrap's gas volume at the specified reservoir phase
	    virtual double getGasVolume (PhaseId phaseId) const;
	    /// return the LeadTrap's oil volume at the specified reservoir phase
	    virtual double getOilVolume (PhaseId phaseId) const;

	 private:
	    const Trap * m_trap;
	    TrapPhase * m_trapPhases[NumPhases];
      };
   }
}

#endif // _INTERFACE_LEADTRAP_H_
