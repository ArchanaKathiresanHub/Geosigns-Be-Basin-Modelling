#ifndef _VOXET_DERIVED_PROPERTY_H_
#define _VOXET_DERIVED_PROPERTY_H_

#include <string>

// Data-access
#include "../Interface/ProjectHandle.h"

using namespace DataAccess;
using namespace Interface;


// Table-io
#include "database.h"

#include "VoxetCalculator.h"
#include "VoxetPropertyGrid.h"

// Forward declaration of VoxetProjectHandle.
class VoxetProjectHandle;

/// The derived-property that is to be computed.
class DerivedProperty {

public :

   DerivedProperty ( Interface::ProjectHandle* cauldronProjectHandle,
                     VoxetProjectHandle*            voxetHandle,
                     database::Record*              record );

   /// The name of the property.
   const std::string& getName () const;

   /// The units of the property.
   const std::string& getUnits () const;

   /// Calculate the property and fill the voxet-property-grid.
   void calculate ( VoxetCalculator&   interpolator,
                    const Snapshot*    snapshot,
                    VoxetPropertyGrid& values,
                    const bool         verbose = false );
   

private :

   // Detault value is in feet/sec
   static const float DefaultWaterVelocity;

   Interface::ProjectHandle* m_cauldronProjectHandle;
   VoxetProjectHandle* m_voxetHandle;
   database::Record*   m_record;

}; 

#endif // _VOXET_DERIVED_PROPERTY_H_
