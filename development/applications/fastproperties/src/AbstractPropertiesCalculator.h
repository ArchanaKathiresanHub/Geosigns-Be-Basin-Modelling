//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTPROPERTIES__ABSTRACT_PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__ABSTRACT_PROPERTIES_CALCULATOR__H


#include "OutputUtilities.h"

// utilities library
#include "LogHandler.h"
#include "StatisticsHandler.h"

#include <petsc.h>

namespace DerivedProperties
{
  class DerivedPropertyManager;
}

typedef vector < double > DoubleVector;

class AbstractPropertiesCalculator {

public :
   AbstractPropertiesCalculator( int aRank );

   virtual ~AbstractPropertiesCalculator();


protected:
   int m_rank;

   PetscLogDouble m_startTime;

   std::string m_projectFileName;

   bool m_primaryPod;        /// load results form the shared directory on the cluster
   bool m_extract2D;         ///< true if 2D primary/derived properties to be calculated and saved
   bool m_no3Dproperties;    ///< true if no 3d properties to be calculated

   bool m_debug;
   bool m_basement;           ///< If true: include the basement
   bool m_all2Dproperties;    ///< If true: computes all 2D derived properties
   bool m_all3Dproperties;    ///< If true: computes all 3D derived properties
   bool m_listProperties;     ///< If true: prints all outputable properties
   bool m_listSnapshots;      ///< If true: prints all snapshots from project file
   bool m_listStratigraphy;   ///< If true: prints all stratigraphy from project file

   StringVector m_propertyNames;
   DoubleVector m_ages;
   StringVector m_formationNames;

   std::string m_activityName;   ///< The name of the current activity producing output values
   std::string m_simulationMode; ///< The name of the last simulation fastcauldron mode
   bool   m_decompactionMode;
   int    m_snapshotsType;  ///< The type of snapshots to calculate derived properties at

   DataModel::PropertyOutputAttribute m_attribute; ///< The output attribute of the properties to convert

   std::set<std::string>  m_basementPropertiesList; ///< List of the properties output for basement

   std::string m_commandLine;

   const DataAccess::Interface::Grid * m_activeGrid; ///< Active output grid

   /// \brief Check command-line parameters
   virtual bool checkParameters() = 0;

   /// \brief Check if the property the formation/surface is allowed to be output
   bool allowOutput ( const std::string & propertyName, const Interface::Formation * formation, const Interface::Surface * surface ) const;

   /// \brief Check if the property the formation/surface is selected to be output in the project file
   Interface::PropertyOutputOption checkTimeFilter3D ( const std::string& name ) const;

   /// \brief Reset project handle active grid to read primary properties in high resolution
   void resetProjectActivityGrid(const DataAccess::Interface::Property * property = 0);

   /// \brief Check if the property should be calculated for basement
   bool isBasementProperty (const std::string& propertyName) const;

   /// \brief Indicates whether or not open HDF file (2D data) for writing
   virtual bool getProperiesActivity() const = 0;

   /// \brief Copy the results from the shared directory to the local directory
   virtual bool copyFiles() = 0;

public:

   GeoPhysics::ProjectHandle& getProjectHandle() const;
   DerivedProperties::DerivedPropertyManager& getPropertyManager() const;

   bool showLists();

   bool startActivity();
   bool finalise ( bool isComplete );
   bool createFrom( DataAccess::Interface::ObjectFactory* factory);

   bool setFastcauldronActivityName();

   bool acquireSnapshots( Interface::SnapshotList & snapshots );
   /// @brief Acquire outputable 3D properties from project handle
   /// @pre -all-3D-properties must be specified as a command line parameter
   void acquireAll3Dproperties();
   /// @brief Acquire outputable 2D properties from project handle
   /// @pre -all-2D-properties must be specified as a command line parameter
   void acquireAll2Dproperties();
   /// @brief Create formation/surface pairs for the properties to be calculated
   void acquireFormationsSurfaces( FormationSurfaceVector & formationSurfaceItems );
   /// @brief Acquire the proprerties list
   void acquireProperties( Interface::PropertyList & properties );
   /// @brief Check if the property should be calculated for basement
   bool allowBasementOutput (const std::string& propertyName3D) const;
   /// @brief Acquire outputable simulator properties from project handle
   /// @pre -genex or -mig must be specified as a command line parameter
   virtual void acquireSimulatorProperties() { return; }

   /// @brief Print all outputable 2D and 3D properties from project handle
   /// @pre -list-properties must be specified as a command line parameter
   void printOutputableProperties ();
   void printListSnapshots ();
   void printListStratigraphy ();

   /// @brief Calculate, convert and output the derived and primary properties
   virtual void calculateProperties( FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots ) = 0;

   virtual bool parseCommandLine ( int argc, char ** argv );
   void showUsage( const char* command, const char* message = 0 );

   void startTimer();

   virtual void writeToHDF() = 0;
   virtual void listXmlProperties() = 0;
   virtual bool hdfonly() const = 0;
   virtual bool listXml() const = 0;

   static void displayTime (const double timeToDisplay, const char * msgToDisplay);
protected:
   void displayProgress (const std::string & fileName, double startTime, const std::string & message);

private:

   std::unique_ptr<GeoPhysics::ProjectHandle> m_projectHandle;
   DerivedProperties::DerivedPropertyManager * m_propertyManager;

};

#endif // FASTPROPERTIES__ABSTRACT_PROPERTIES_CALCULATOR__H
