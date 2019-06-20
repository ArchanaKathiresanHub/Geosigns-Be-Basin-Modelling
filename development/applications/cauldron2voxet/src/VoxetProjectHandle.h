#ifndef _VOXET_PROJECT_HANDLE_H_
#define _VOXET_PROJECT_HANDLE_H_

#include <vector>
#include "ProjectHandle.h"
#include "Grid.h"

using namespace DataAccess;
using namespace Interface;

#include "GridDescription.h"

// Forward declarations.
class CauldronProperty;
typedef std::vector<CauldronProperty*> CauldronPropertyList;

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
};

Interface::ProjectHandle* VoxetProjectHandle::getCauldronProjectHandle ()
{
   return m_cauldronProjectHandle;
}

#endif // _VOXET_PROJECT_HANDLE_H_
