#ifndef FASTCAULDRON_DERIVED_PROPERTIES_CALCULATOR_HH
#define FASTCAULDRON_DERIVED_PROPERTIES_CALCULATOR_HH

#include <string>
#include <sstream>

#include "FastcauldronSimulator.h"
#include "PropertyManager.h"
#include "timefilter.h"
#include "propinterface.h"

#include "DerivedPropertyManager.h"
#include "OutputUtilities.h"

using namespace DerivedProperties;


/// \brief Calculates derived properties and save them to disk

class DerivedPropertiesCalculator {

public :
   DerivedPropertiesCalculator( AppCtx * appctx, const PropListVec& propertyVolumeNames, const PropListVec& propertyMapNames );

   ~DerivedPropertiesCalculator();

 
   /// \brief Calculate the derived properties
   bool compute();

   /// \brief Calculate and save to disk the derived properties for the formation/surface pairs for the snapshot ages
   ///
   /// \param [in]  formationSurfaceItems  List of formations/surfaces
   /// \param [in]  snapshots              List of snapshots to be calculated
   /// \param [in]  properties             List of properties to be calculated.
   /// \param [in] allOutputPropertyValues List of allocated propertyValues 
   /// \pre snapshot list is not null 
   /// \pre formationSurfaceItems list is not null 

   bool calculateProperties( FormationSurfaceVector& formationSurfaceItems,  SnapshotList & snapshots, 
                             DataAccess::Interface::PropertyList & properties,
                             DerivedProperties::SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues );

   /// \brief Create a list of property names (strings)
   ///
   /// \param [in] propertyNames  Vector of property IDs
   void acquirePropertyNames( const PropListVec& propertyNames );
 
private:

   /// \brief The Fastcaulsron Simulator
   FastcauldronSimulator * m_simulator;

   /// \brief The Application context
   AppCtx * m_appctx;

   /// \brief The Derived properties manager
   DerivedPropertyManager * m_propertyManager;
 
   /// \brief Vector of the property names to be computed
   StringVector m_propertyNames;

   bool m_debug;
   int m_rank;
  
   /// \brief Identificators of the current fastcauldron simulation mode
   bool m_decompactionMode;

   /// \brief Print the list of the propety names
   void printNames();

   /// \brief Allocate property calculators for the list of formations/surfaces for the snapshots and the properties
   ///
   /// \param [in]  formationSurfaceItems    List of formations/surfaces
   /// \param [in]  snapshots                List of snapshots to be calculated
   /// \param [in]  properties               List of properties to be calculated.
   /// \param [out] allOutputPropertyValues  On exit will contain the list of allocated propertyValues 
   /// \pre snapshot list is not null 
   /// \pre formationSurfaceItems list is not null 
   void allocateAllProperties( const FormationSurfaceVector & formationSurfacePairs, 
                               DataAccess::Interface::PropertyList properties, 
                               const SnapshotList & snapshots, 
                               DerivedProperties::SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues);
};

#endif // FASTCAULDRON_DERIVED_PROPERTIES_CALCULATOR_HH
