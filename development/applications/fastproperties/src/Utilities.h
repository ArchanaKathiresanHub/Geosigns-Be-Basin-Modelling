#ifndef FAST_PROPERTIES__UTILITIES_HH
#define FAST_PROPERTIES__UTILITIES_HH

#include "OutputUtilities.h"

#include "VisualizationAPI.h"
#include "ExportToXML.h"
#include "ImportProjectHandle.h"
#include "VisualizationIO_projectHandle.h"
#include "VisualizationIO_native.h"

using namespace std;
using namespace DataAccess;
using namespace Interface;
using namespace CauldronIO;

namespace DerivedProperties {

   std::shared_ptr<SnapShot> getSnapShot(std::shared_ptr< CauldronIO::Project>& project, const double age);
   std::shared_ptr< CauldronIO::Surface > getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, const std::string &name );
   std::shared_ptr< CauldronIO::Surface > getSurface( std::shared_ptr< CauldronIO::SnapShot>& snapshot, std::shared_ptr< CauldronIO::Formation>& formation );
   
   bool createVizSnapshotResultPropertyValueMap (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                   GeoPhysics::ProjectHandle* projectHandle, 
                                                   OutputPropertyValuePtr propertyValue, 
                                                   const Snapshot* snapshot, const Interface::Formation * formation,
                                                   const Interface::Surface * surface,
                                                   vector<float>& inData );
   
   bool createVizSnapshotResultPropertyValueContinuous (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                          GeoPhysics::ProjectHandle* projectHandle, 
                                                          OutputPropertyValuePtr propertyValue, 
                                                          const Snapshot* snapshot, const Interface::Formation * formation,
                                                          std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                          vector<float>& inData);
   
   bool  createVizSnapshotResultPropertyValueDiscontinuous (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                              GeoPhysics::ProjectHandle* projectHandle, 
                                                              OutputPropertyValuePtr propertyValue, 
                                                              const Snapshot* snapshot, const Interface::Formation * formation,
                                                              std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                              vector<float>& inData);
   
   void  createVizSnapshotFormationData( std::shared_ptr<CauldronIO::Project> vizProject,
                                         GeoPhysics::ProjectHandle* projectHandle, 
                                         const Snapshot * snapshot, const FormationSurface & formationItem, 
                                         DataAccess::Interface::PropertyList & properties,
                                         SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues,
                                         std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                         vector<float>& inData );
   
   bool createVizSnapshotResultPropertyValue (  std::shared_ptr<CauldronIO::Project> vizProject,
                                                GeoPhysics::ProjectHandle* projectHandle, 
                                                OutputPropertyValuePtr propertyValue, 
                                                const Snapshot* snapshot, const Interface::Formation * formation,
                                                const Interface::Surface * surface,
                                                std::shared_ptr<CauldronIO::FormationInfoList> &formInfoList,
                                                vector<float>& inData);
   
   std::shared_ptr<CauldronIO::FormationInfoList>  getDepthFormations(GeoPhysics::ProjectHandle* projectHandle, const Snapshot* snapShot);
   void updateConstantValue( std::shared_ptr< CauldronIO::SnapShot>& snapshot );
   void updateVolumeDataConstantValue( shared_ptr<CauldronIO::VolumeData>& volDataNew );
   CauldronIO::SnapShotKind getSnapShotKind(const Interface::Snapshot* snapShot);
   void saveVizSnapshot( const std::shared_ptr<SnapShot>& snapShot, const std::string& absPath);
   void collectVolumeData( const std::shared_ptr<SnapShot>& snapShot, vector<float>& inData );
   void addVolume( GeoPhysics::ProjectHandle* projectHandle, OutputPropertyValuePtr propertyValue, unsigned int firstK, unsigned int lastK,  unsigned int numK, vector<float> & inData );
}

#endif
