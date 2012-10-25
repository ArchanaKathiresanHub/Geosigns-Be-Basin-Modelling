#ifndef _VOXET_CAULDRON_PROPERTY_H_
#define _VOXET_CAULDRON_PROPERTY_H_

#include <string>

// Data-access
#include "../Interface/ProjectHandle.h"
#include "Property.h"

using namespace DataAccess;
using namespace Interface;

// Table-io
#include "database.h"

// // Voxet-project
// #include "VoxetProjectHandle.h"

// Forward declaration of VoxetProjectHandle.
class VoxetProjectHandle;

/// A property that is computed and output by Cauldron.
class CauldronProperty {

public :

   CauldronProperty ( Interface::ProjectHandle* cauldronProjectHandle,
                      VoxetProjectHandle*            voxetHandle,
                      database::Record*              record );

   /// The name of the property.
   const std::string& getCauldronName () const;

   /// The name of the property.
   const std::string& getVoxetName () const;

   /// The DAL cauldron property object.
   const Property* getProperty () const;

   /// Should this property be output in the voxet file.
   bool getVoxetOutput () const;

   /// What is the conversion factor for this property.
   float getConversionFactor () const;

   /// What are the units of this property.
   const std::string& getUnits () const;

private :

   Interface::ProjectHandle* m_cauldronProjectHandle;
   VoxetProjectHandle*            m_voxetHandle;
   database::Record*              m_record;


}; 

#endif // _VOXET_CAULDRON_PROPERTY_H_
