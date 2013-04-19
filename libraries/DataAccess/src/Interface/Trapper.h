#ifndef _INTERFACE_TRAPPER_H_
#define _INTERFACE_TRAPPER_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// Trapper Class
      /// A Trapper object models a hydrocarbon storage area in a Reservoir
      class Trapper : public DAObject
      {
	 public:
	    Trapper (ProjectHandle * projectHandle, database::Record * record);
	    ~Trapper (void);

	    /// See if this Trapper matches the given conditions
	    bool matchesConditions (const Reservoir * reservoir, const Snapshot * snapshot, unsigned int id, unsigned int persistentId);

	    /// Return the persistent id of this Trapper
	    virtual unsigned int getPersistentId (void) const;

	    /// Return the id of this Trapper
	    virtual unsigned int getId (void) const;

	    /// Return the Snapshot at which this Trapper was computed
	    virtual const Snapshot * getSnapshot (void) const;

	    /// Return the Reservoir containing this Trapper
	    virtual const Reservoir * getReservoir (void) const;

	    /// Return the trappers that spill to this trap
	    virtual const TrapperList * getUpstreamTrappers (void) const;

	    /// Return the trap this trap spills to
	    virtual const Trapper * getDownstreamTrapper (void) const;

	    /// Return the real world position of this Trapper
	    virtual void getPosition (double & x, double & y) const;

        /// Return the real world position of this Trapper
	    virtual void getSpillPointPosition (double & x, double & y) const;

	    /// Return the grid position of this Trapper
	    virtual void getGridPosition (unsigned int & i, unsigned int & j) const;

	    /// return the mass of the given component in this Trapper
	    virtual double getMass (ComponentId componentId) const;
	    virtual double getMass (PhaseId rcPhaseId, ComponentId componentId) const;
	    virtual double getMass (PhaseId rcPhaseId, PhaseId stPhaseId, ComponentId componentId) const;

	    /// return the mass of the given component in this Trap
	    virtual double getMass (const string & stockTankPhaseName, const string & componentName) const;

	    /// return the mass of the given phase in this Trapper
	    virtual double getMass (PhaseId rcPhaseId) const;
	    /// return the mass of the given phase combo in this Trapper
	    virtual double getMass (PhaseId rcPhaseId, PhaseId stPhaseId) const;

	    /// return the volume of the given phase in this Trapper
	    virtual double getVolume (PhaseId rcPhaseId) const;
	    /// return the volume of the given phase combo in this Trapper
	    virtual double getVolume (PhaseId rcPhaseId, PhaseId stPhaseId) const;
	    /// return the Volume of the given phase in this Trap
	    virtual double getVolume (const string & phaseName) const;

	    /// return the Viscosity of the given phase in this Trap
	    virtual double getViscosity (const string & phaseName) const;
	    /// return the Viscosity of the given phase in this Trapper
	    virtual double getViscosity (PhaseId rcPhaseId) const;
	    /// return the viscosity of the given phase combo in this Trapper
	    virtual double getViscosity (PhaseId rcPhaseId, PhaseId stPhaseId) const;

	    /// return the Density of the given phase in this Trap
	    virtual double getDensity (const string & phaseName) const;
	    /// return the Density of the given phase in this Trapper
	    virtual double getDensity (PhaseId rcPhaseId) const;
	    /// return the density of the given phase combo in this Trapper
	    virtual double getDensity (PhaseId rcPhaseId, PhaseId stPhaseId) const;

	    /// return the InterfacialTension of the given phase in this Trap
	    virtual double getInterfacialTension (const string & phaseName) const;
	    /// return the interfacial tension of the given phase in this Trapper
	    virtual double getInterfacialTension (PhaseId rcPhaseId) const;

	    /// return the CriticalTemperature of the given phase in this Trap
	    virtual double getCriticalTemperature (const string & phaseName) const;
	    /// return the critical temperature of the given phase in this Trapper
	    virtual double getCriticalTemperature (PhaseId rcPhaseId) const;

	    /// return the CEP of the given phase in this Trap
	    virtual double getCEP (const string & phaseName) const;
	    /// return the capillary entry pressure of the given phase in this Trapper
	    virtual double getCEP (PhaseId rcPhaseId) const;

	    /// return the oil API of this Trapper
	    virtual double getOilAPI (void) const;

	    /// return the cgr this Trapper
	    virtual double getCGR (void) const;

	    /// return the GOR this Trapper
	    virtual double getGOR (void) const;

	    /// return the fracture pressure this Trapper
	    virtual double getFracturePressure (void) const;

	    /// return the Buoyancy this Trapper
	    virtual double getBuoyancy (void) const;

	    /// return the capacity of this Trapper
	    virtual double getCapacity (void) const;

	    /// return the Trapper's depth
	    virtual double getDepth (void) const;
	    /// return the Trapper's spill depth
	    virtual double getSpillDepth (void) const;
	    /// return the Trapper's temperature
	    virtual double getTemperature (void) const;
	    /// return the Trapper's pressure
	    virtual double getPressure (void) const;
	    /// return the Trapper's gas-oil contact depth
	    virtual double getGOC (void) const;
	    /// return the Trapper's oil-water contact depth
	    virtual double getOWC (void) const;
	    /// return the Trap's surface at the oil-water contact depth
	    virtual double getWCSurface (void) const;
	    /// return the Trapper's Permeability
	    virtual double getPermeability (void) const;
	    /// return the Trapper's Seal Permeability
	    virtual double getSealPermeability (void) const;
	    /// return the Trapper's Porosity
	    virtual double getPorosity (void) const;
            
	    void addUpstreamTrapper (Trapper * trapper);
	    void setDownstreamTrapper (Trapper * trapper);
	    
	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

	    void setReservoir (const Reservoir * reservoir);
	    void setSnapshot (const Snapshot * snapshot);

	 private:
	    const Reservoir * m_reservoir;
	    const Snapshot * m_snapshot;

	    MutableTrapperList m_upstreamTrappers;
	    const Trapper * m_downstreamTrapper;
      };
   }
}

#endif // _INTERFACE_TRAPPER_H_
