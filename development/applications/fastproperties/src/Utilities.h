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
   void listProperties( const std::shared_ptr<SnapShot>& snapShot, std::shared_ptr<Project> &project);

   std::shared_ptr<CauldronIO::FormationInfoList>  getDepthFormations(GeoPhysics::ProjectHandle& projectHandle, const Snapshot* snapShot);
   void updateVolumeDataConstantValue( shared_ptr<CauldronIO::VolumeData>& volDataNew );
   CauldronIO::SnapShotKind getSnapShotKind(const Interface::Snapshot* snapShot);
   void saveVizSnapshot( const std::shared_ptr<SnapShot>& snapShot, const std::string& absPath);
   void addVolume( GeoPhysics::ProjectHandle& projectHandle, OutputPropertyValuePtr propertyValue, unsigned int firstK, unsigned int lastK,  unsigned int numK, vector<float> & inData );
   void minmax_op( float *invec, float *inoutvec, int *len, MPI_Datatype *datatype );
   void minmaxint_op( int *invec, int *inoutvec, int *len, MPI_Datatype *datatype );
}

#endif
