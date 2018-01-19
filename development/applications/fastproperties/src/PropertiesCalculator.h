//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef FASTPROPERTIES__PROPERTIES_CALCULATOR__H
#define FASTPROPERTIES__PROPERTIES_CALCULATOR__H

//std library
#include <string>
#include <sstream>
#include <vector>
#include <utility>
#include <stdexcept>

//DataAccess library
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"
#include "Interface/Formation.h"
#include "Interface/ObjectFactory.h"

#include "Interface/Property.h"
#include "Interface/PropertyValue.h"

#include "PropertyAttribute.h"
#include "AbstractPropertyManager.h"
#include "DerivedPropertyManager.h"
#include "OutputUtilities.h"

#include "GeoPhysicsObjectFactory.h"
#include "GeoPhysicsProjectHandle.h"
#include "OutputPropertyValue.h"
#include "FormationOutputPropertyValue.h"
#include "FormationMapOutputPropertyValue.h"
#include "SurfaceOutputPropertyValue.h"

#include "VisualizationAPI.h"
#include "ExportToXML.h"
#include "ImportProjectHandle.h"

using namespace std;

using namespace DataAccess;
using namespace Interface;
using namespace DerivedProperties;

typedef vector < double > DoubleVector;

template <class T>
T Min( T x, T y )
{
   return ( x < y ? x : y );
}

template <class T>
T Max( T x, T y )
{
   return ( x > y ? x : y );
}

template <class T>
void Swap( T & x, T & y )
{
   T v;
   v = x;
   x = y;
   y = v;
}

class PropertiesCalculator {

   const std::string xmlExt = ".xml";

public :
   PropertiesCalculator( int aRank );

   virtual ~PropertiesCalculator();

   int m_rank;

private:
   PetscLogDouble m_startTime;

   std::shared_ptr<ProjectHandle> m_sharedProjectHandle;
   std::shared_ptr<CauldronIO::Project> m_vizProject;
   GeoPhysics::ProjectHandle* m_projectHandle;
   DerivedPropertyManager * m_propertyManager;
   CauldronIO::ExportToXML * m_export;
   pugi::xml_document m_doc;
   pugi::xml_node m_pt;
   pugi::xml_node m_snapShotNodes;

   bool m_convert;
   bool m_vizFormat;
   bool m_vizFormatHDF;
   bool m_vizFormatHDFonly;
   bool m_vizListXml;
   bool m_primaryPod;
   bool m_minorPrimary;
   bool m_extract2D;         ///< true if 2D primary/derived properties to be calculated and saved
   bool m_no3Dproperties;    ///< true if no 3d properties are defined to be calculated

   bool m_debug;
   bool m_basement;
   bool m_copy;
   bool m_all2Dproperties;    ///< If true: computes all 2D derived properties
   bool m_all3Dproperties;    ///< If true: computes all 3D derived properties
   bool m_listProperties;     ///< If true: prints all outputable properties
   bool m_listSnapshots;      ///< If true: prints all snapshots from project file
   bool m_listStratigraphy;   ///< If true: prints all stratigraphy from project file
   bool m_projectProperties;

   string m_projectFileName;
   string m_fileNameXml;

   StringVector m_propertyNames;
   DoubleVector m_ages;
   StringVector m_formationNames;

   string m_activityName;   ///< The name of the current activity producing output values
   string m_simulationMode; ///< The name of the last simulation fastcauldron mode
   bool   m_decompactionMode;
   int    m_snapshotsType;  ///< The type of snapshots to calculate derived properties at

   std::vector<float> m_data; ///< Buffer to convert a property to visualization format
   std::shared_ptr<CauldronIO::FormationInfoList> m_formInfoList; ///< List of formations grid info

   std::string  m_commandLine;

   MPI_Op m_op; ///< Custom MPI operation to find maximum and minimum of sediment properties

   /// \brief Check if the property the formation/surface is allowed to be output
   bool allowOutput ( const string & propertyName, const Interface::Formation * formation, const Interface::Surface * surface ) const;
   /// \brief Check if the property the formation/surface is selected to be output in the project file
   PropertyOutputOption checkTimeFilter3D ( const string & name ) const;

   /// \brief Create visualization format project: import projectHandle and create xml file
   void createXML();
   /// \brief Import projectHandle into visualization format
   std::shared_ptr<CauldronIO::Project> createStructureFromProjectHandle( bool verbose );
   
   /// \brief Finish export to visualization format: add project related information to xml file and save it on disk
   void saveXML();

   /// \brief Create a property map in visualization format
   bool createVizSnapshotResultPropertyValueMap ( OutputPropertyValuePtr propertyValue, 
                                                  const Snapshot* snapshot, const Interface::Formation * formation,
                                                  const Interface::Surface * surface );
   
   /// \brief Create a continuous property in visualization format
   bool createVizSnapshotResultPropertyValueContinuous ( OutputPropertyValuePtr propertyValue, 
                                                         const Snapshot* snapshot, const Interface::Formation * formation );
   
   /// \brief Create a discontinuous property in visualization format
   bool  createVizSnapshotResultPropertyValueDiscontinuous ( OutputPropertyValuePtr propertyValue, 
                                                             const Snapshot* snapshot, const Interface::Formation * formation );
   
   /// \brief Create list of properties for a formation/surface pair at a snapshot age in visualization format
   void  createVizSnapshotFormationData( const Snapshot * snapshot, const FormationSurface & formationItem, 
                                         DataAccess::Interface::PropertyList & properties,
                                         SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues );
   
   /// \brief Create a property for a formation/surface pair at a snapshot age in visualization format
   bool createVizSnapshotResultPropertyValue ( OutputPropertyValuePtr propertyValue, 
                                               const Snapshot* snapshot, const Interface::Formation * formation,
                                               const Interface::Surface * surface );

   /// \brief Calculate k-range (offsets in depth) for all stratigraphy formations.
   void updateFormationsKRange();

   /// \brief Collect continuous volume on rank 0
   void collectVolumeData( const std::shared_ptr<CauldronIO::SnapShot>& snapShot );
   
public:

   GeoPhysics::ProjectHandle* getProjectHandle() const;
   DerivedPropertyManager * getPropertyManager() const;

   bool showLists();

   bool startActivity();
   bool finalise ( bool isComplete );
   bool CreateFrom( DataAccess::Interface::ObjectFactory* factory);

   bool setFastcauldronActivityName();

   bool acquireSnapshots( SnapshotList & snapshots );
   /// @brief Acquire outputable 3D properties from project handle
   /// @pre -all-3D-properties must be specified as a command line parameter
   void acquireAll3Dproperties();
   /// @brief Acquire outputable 2D properties from project handle
   /// @pre -all-2D-properties must be specified as a command line parameter
   void acquireAll2Dproperties();
   void acquireFormationsSurfaces( FormationSurfaceVector & formationSurfaceItems );
   void acquireProperties( Interface::PropertyList & properties );

   /// @brief Print all outputable 2D and 3D properties from project handle
   /// @pre -list-properties must be specified as a command line parameter
   void printOutputableProperties ();
   void printListSnapshots ();
   void printListStratigraphy ();

   void calculateProperties( FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots );
   
   void convertToVisualizationIO();
   void updateVizSnapshotsConstantValue();

   bool convert() const;
   bool hdfonly() const;
   bool vizFormat() const;
   bool listXml() const;

   void writeToHDF();
   void listXmlProperties();
   bool parseCommandLine ( int argc, char ** argv );
   void showUsage( const char* command, const char* message = 0 );

   bool copyFiles();

   void startTimer();
 
};

#endif // FASTPROPERTIES__PROPERTIES_CALCULATOR__H
