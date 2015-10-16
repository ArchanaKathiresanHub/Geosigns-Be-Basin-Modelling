/* File : VisualizationAPI.i */
 
%module CauldronIO

%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"
%include "std_pair.i"
%include "boost_shared_ptr.i"

%shared_ptr(CauldronIO::SnapShot)
%shared_ptr(CauldronIO::Surface)
%shared_ptr(CauldronIO::Volume)
%shared_ptr(CauldronIO::DiscontinuousVolume)
%shared_ptr(CauldronIO::Map)
%shared_ptr(CauldronIO::Formation)
%shared_ptr(CauldronIO::Property)
%shared_ptr(CauldronIO::FormationVolume)
%shared_ptr(CauldronIO::Project)

%{
/* Includes the header in the wrapper code */
#include "VisualizationAPI.h"
#include "ImportExport.h"
%}

%include "VisualizationAPI.h"
%include "ImportExport.h"

/* some templates */
%template(SnapShotList) std::vector < boost::shared_ptr<CauldronIO::SnapShot > >;
%template(FormationVolume) std::pair<boost::shared_ptr<CauldronIO::Formation>, boost::shared_ptr<CauldronIO::Volume> >;
%template(FormationVolumeList) std::vector<boost::shared_ptr<CauldronIO::FormationVolume > >;
%template(SurfaceList) std::vector<boost::shared_ptr<CauldronIO::Surface > > ;
%template(VolumeList) std::vector<boost::shared_ptr<CauldronIO::Volume > > ;
%template(DiscontinuousVolumeList) std::vector<boost::shared_ptr<CauldronIO::DiscontinuousVolume > > ;


