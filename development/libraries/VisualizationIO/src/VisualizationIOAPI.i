/* File : VisualizationIOAPI.i */
 
%module VisualizationIOAPI

%{
/* Includes the header in the wrapper code */
#include "VisualizationAPIFwDecl.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"
#include "ImportFromXML.h"
#include "FilePath.h"
#include "FolderPath.h"
%}

%include "../../swig-common/StdHelper.i"
%include "typemaps.i"
%include "std_shared_ptr.i"

/* Use an IntPtr for float* data */
%typemap(ctype)  float * "float *"
%typemap(imtype) float * "System.IntPtr"
%typemap(cstype) float * "System.IntPtr"
%typemap(csin)   float * "$csinput"
%typemap(in)     float * %{ $1 = $input; %}
%typemap(out)    float * %{ $result = $1; %}
%typemap(csout, excode=SWIGEXCODE)  float * { $excode return $imcall; }

/* some output reference types */
namespace CauldronIO
{
    %apply float&  OUTPUT { float& spillPointPosX };
    %apply float&  OUTPUT { float& spillPointPosY };
    %apply float&  OUTPUT { float& positionPosX };
    %apply float&  OUTPUT { float& positionPosY };
    %apply int&    OUTPUT { int& k_range_start };
    %apply int&    OUTPUT { int& k_range_end };
}

%shared_ptr(CauldronIO::SnapShot)
%shared_ptr(CauldronIO::Surface)
%shared_ptr(CauldronIO::Volume)
%shared_ptr(CauldronIO::VolumeData)
%shared_ptr(CauldronIO::SurfaceData)
%shared_ptr(CauldronIO::VisualizationIOData)
%shared_ptr(CauldronIO::ReferenceVolume)
%shared_ptr(CauldronIO::Formation)
%shared_ptr(CauldronIO::Property)
%shared_ptr(CauldronIO::Project)
%shared_ptr(CauldronIO::Trapper)
%shared_ptr(CauldronIO::Trap)
%shared_ptr(CauldronIO::MapNative)
%shared_ptr(CauldronIO::ReferenceMap)
%shared_ptr(CauldronIO::VolumeDataNative)
%shared_ptr(CauldronIO::Geometry2D)
%shared_ptr(CauldronIO::Geometry3D)
%shared_ptr(CauldronIO::Reservoir)
%shared_ptr(CauldronIO::HDFinfo)
%shared_ptr(CauldronIO::MigrationEvent)

%ignore CauldronIOException;
%include "VisualizationAPIFwDecl.h"
%include "VisualizationAPI.h"
%include "VisualizationIO_native.h"
%include "ImportFromXML.h"

/* some templates */
%template(SnapShotList)                   std::vector < std::shared_ptr<CauldronIO::SnapShot > >;
%template(FormationVolume)                std::pair<std::shared_ptr<const CauldronIO::Formation>, std::shared_ptr<CauldronIO::Volume> >;
%template(FormationVolumeList)            std::vector< CauldronIO::FormationVolume >;
%template(SurfaceList)                    std::vector<std::shared_ptr<CauldronIO::Surface > > ;
%template(VolumeList)                     std::vector<std::shared_ptr<CauldronIO::Volume > > ;
%template(TrapperList)                    std::vector<std::shared_ptr<CauldronIO::Trapper > > ;
%template(TrapList)                       std::vector<std::shared_ptr<CauldronIO::Trap> >;
%template(PropertyList)                   std::vector<std::shared_ptr<const CauldronIO::Property > > ;
%template(PropertySurfaceData)            std::pair<std::shared_ptr<const CauldronIO::Property>, std::shared_ptr<CauldronIO::SurfaceData> >;
%template(PropertySurfaceDataList)        std::vector < CauldronIO::PropertySurfaceData > ;
%template(ReservoirList)                  std::vector<std::shared_ptr<const CauldronIO::Reservoir> >  ;
%template(FormationList)                  std::vector<std::shared_ptr<CauldronIO::Formation> > ;
%template(PropertyVolumeData)             std::pair<std::shared_ptr<const CauldronIO::Property>, std::shared_ptr<CauldronIO::VolumeData> >;
%template(PropertyVolumeDataList)         std::vector<CauldronIO::PropertyVolumeData > ;
%template(HDFinfoList)                    std::vector<std::shared_ptr<CauldronIO::HDFinfo> >;
%template(GeometryList)                   std::vector<std::shared_ptr<const CauldronIO::Geometry2D> >;
%template(MigrationEventList)             std::vector<std::shared_ptr<CauldronIO::MigrationEvent> >;
%template(StratigraphyTableEntryList)     std::vector<CauldronIO::StratigraphyTableEntry>;
