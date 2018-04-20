//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTPROPERTIES__HDF_PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__HDF_PROPERTIES_CALCULATOR__H

#include "AbstractPropertiesCalculator.h"

class HdfPropertiesCalculator : public AbstractPropertiesCalculator {

public :
   HdfPropertiesCalculator( int aRank );

   virtual ~HdfPropertiesCalculator() = default;

   bool getProperiesActivity() const;
private:

   bool m_copy;
   bool m_projectProperties;

    /// \brief Check command-line parameters consistency
   bool checkParameters();

   /// \brief Check if the property the formation/surface is allowed to be output
   bool allowOutput ( const string & propertyName, const Interface::Formation * formation, const Interface::Surface * surface ) const;

   /// \brief Check if the property the formation/surface is selected to be output in the project file
   PropertyOutputOption checkTimeFilter3D ( const string & name ) const;

public:

   void calculateProperties( FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots );

   bool parseCommandLine ( int argc, char ** argv );

   bool copyFiles();
 
   void writeToHDF();
   void listXmlProperties();
   bool hdfonly() const;
   bool listXml() const;
  
};

inline bool HdfPropertiesCalculator::getProperiesActivity () const {
   return false;
}

inline void HdfPropertiesCalculator::writeToHDF() {
   return;
}

inline void HdfPropertiesCalculator::listXmlProperties()  {
   return;
}

inline bool HdfPropertiesCalculator::hdfonly() const {
   return false;
}

inline bool HdfPropertiesCalculator::listXml() const {
   return false;
}

#endif // FASTPROPERTIES__HDF_PROPERTIES_CALCULATOR__H
