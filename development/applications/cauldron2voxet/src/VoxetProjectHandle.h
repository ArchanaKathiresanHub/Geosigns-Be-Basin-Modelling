#ifndef _VOXET_PROJECT_HANDLE_H_
#define _VOXET_PROJECT_HANDLE_H_

#include <vector>
#include "../Interface/ProjectHandle.h"
#include "Grid.h"

using namespace DataAccess;
using namespace Interface;

#include "GridDescription.h"

// Forward declarations.
class CauldronProperty;
typedef std::vector<CauldronProperty*> CauldronPropertyList;

#ifdef ENABLE_CAULDRON2VOXET_DERIVED_PROPERTIES
class DerivedProperty;
class DerivedPropertyFunction;
class DerivedPropertyFormationFunction;

typedef std::vector<DerivedProperty*> DerivedPropertyList;
typedef std::vector<DerivedPropertyFunction*> DerivedPropertyFunctionList;
typedef std::vector<DerivedPropertyFormationFunction*> DerivedPropertyFormationFunctionList;
#endif

/// \brief Name of the formation representing the water-column.
///
/// This is not strictly a Cauldron formation name.
const std::string WaterColumnFormationName = "WaterColumn";

class VoxetProjectHandle {

public :

   VoxetProjectHandle ( const std::string& voxetProjectFileName,
                        Interface::ProjectHandle*     projectHandle );


   CauldronPropertyList::iterator cauldronPropertyBegin ();

   CauldronPropertyList::iterator cauldronPropertyEnd ();

   double getSnapshotTime () const;

#ifdef ENABLE_CAULDRON2VOXET_DERIVED_PROPERTIES

   DerivedProperty* getDerivedProperty ( const std::string& propertyName ) const;

   const DerivedPropertyList& getDerivedProperties () const;

   DerivedPropertyList::iterator getDerivedPropertyBegin ();

   DerivedPropertyList::iterator getDerivedPropertyEnd ();


   DerivedPropertyFunction* getDerivedPropertyFunction ( const std::string& functionName ) const;

   DerivedPropertyFunctionList::iterator getDerivedPropertyFunctionBegin ();

   DerivedPropertyFunctionList::iterator getDerivedPropertyFunctionEnd ();

   DerivedPropertyFunctionList* getDerivedPropertyFunctions ( const DerivedProperty* property ) const;

   /// \brief Get a pointer to the derived-property-formation-function for the specified derived-property.
   DerivedPropertyFormationFunction* getWaterColumnDerivedPropertyFormationFunction ( const DerivedProperty* property ) const;


   DerivedPropertyFormationFunctionList* getDerivedPropertyFormationFunctions ( const DerivedProperty* property ) const;

   DerivedPropertyFormationFunctionList::iterator derivedPropertyFormationFunctionBegin ();

   DerivedPropertyFormationFunctionList::iterator derivedPropertyFormationFunctionEnd ();
#endif

   inline Interface::ProjectHandle* getCauldronProjectHandle ();

   const GridDescription& getGridDescription () const;

   bool isConsistent () const;

private :

   void loadVoxetGrid ( const Interface::Grid* cauldronGrid );
   void loadSnapshotTime ();
   void loadCauldronProperties ();

   const std::string m_voxetProjectFileName;

   Interface::ProjectHandle* m_cauldronProjectHandle;

   double m_snapshotTime;

   database::DataSchema* m_voxetSchema;
   database::Database*   m_database;
   GridDescription* m_gridDescription;
   CauldronPropertyList        m_cauldronProperties;

#ifdef ENABLE_CAULDRON2VOXET_DERIVED_PROPERTIES
   void loadDerivedProperties ();
   void loadDerivedPropertyFunctions ();
   void loadDerivedPropertyFormationFunctions ();
   DerivedPropertyList         m_derivedProperties;
   DerivedPropertyFunctionList m_derivedPropertyFunctions;
   DerivedPropertyFormationFunctionList m_derivedPropertyFormationFunctions;
#endif
};

Interface::ProjectHandle* VoxetProjectHandle::getCauldronProjectHandle ()
{
   return m_cauldronProjectHandle;
}

#endif // _VOXET_PROJECT_HANDLE_H_
