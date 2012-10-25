#ifndef _INTERFACE_RESERVOIR_H_
#define _INTERFACE_RESERVOIR_H_

#include "Interface/DAObject.h"

#include "Interface/Interface.h"

#include <map>

namespace DataAccess
{
   namespace Interface
   {

      /// Reservoir Class
      /// A Reservoir object models a carrier bed in a Formation.
      class Reservoir : public DAObject
      {
	 public:
	    Reservoir (ProjectHandle * projectHandle, database::Record * record);
	    ~Reservoir (void);


	    /// Return the name of this Reservoir
	    const string & getName (void) const;

	    /// Return the minimum capacity for the traps of this Reservoir
	    double getTrapCapacity (void) const;

	    /// Return the mangled name of this Reservoir
	    virtual const string & getMangledName (void) const;

	    /// Return the name of the Formation containing this Reservoir
	    const string & getFormationName (void) const;

	    /// set the Formation of this Reservoir
	    void setFormation (const Formation * formation);

	    /// Return the Formation containing this Reservoir
	    const Formation * getFormation (void) const;

	    /// Return the ActivityMode, one of "AlwaysActive", "NeverActive", "ActiveFrom"
	    virtual const string & getActivityMode (void) const;

#if 0
	    /// Determine whether the reservoir is active at the given snapshot
	    virtual bool isActive (const INTERFACE::Snapshot * snapshot) const;
#endif

	    /// Return the time at which this Reservoir becomes active.
	    /// Only valid if ActivityMode == "ActiveFrom".
	    virtual double getActivityStart (void) const;

	    /// Return whether vertical migration blocking into this reservoir is enabled
            bool isBlockingEnabled (void) const;
            /// return the vertical migration blocking permeability threshold.
            double getBlockingPermeability (void) const;

	    /// Return whether diffusion takes place during migration
	    virtual bool isDiffusionOn (void) const;

	    /// Return whether biodegradation takes place during migration
	    virtual bool isBioDegradationOn (void) const;

	    /// Return whether OTGC takes place during migration
	    virtual bool isOilToGasCrackingOn (void) const;

	    /// Return the (GridMap) value of one of this Reservoir's attributes
	    virtual const GridMap * getMap (ReservoirMapAttributeId attributeId) const;

	    /// load a map
	    GridMap * loadMap (ReservoirMapAttributeId attributeId) const;

	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

	    void setTrappersUpAndDownstreamConnected (double time);
	    bool trappersAreUpAndDownstreamConnected (double time) const;

	 private:
	    string m_mangledName;

	    const Formation * m_formation;
	    static const string s_MapAttributeNames[];

	    map<double, bool> m_trappersUpAndDownstreamConnected;
      };
   }
}

#endif // _INTERFACE_RESERVOIR_H_
