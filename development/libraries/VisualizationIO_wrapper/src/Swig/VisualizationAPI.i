/* File : VisualizationAPI.i */
 
%module CauldronIO

%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"
%include "std_pair.i"
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
    %apply unsigned int& OUTPUT { unsigned int& k_range_start };
    %apply unsigned int& OUTPUT { unsigned int& k_range_end };
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
%shared_ptr(CauldronIO::MapNative)
%shared_ptr(CauldronIO::ReferenceMap)
%shared_ptr(CauldronIO::VolumeDataNative)
%shared_ptr(CauldronIO::Geometry2D)
%shared_ptr(CauldronIO::Geometry3D)
%shared_ptr(CauldronIO::Reservoir)
%shared_ptr(CauldronIO::HDFinfo)

%{
/* Includes the header in the wrapper code */
#include "VisualizationAPIFwDecl.h"
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"
#include "ImportExport.h"
#include "FilePath.h"
#include "FolderPath.h"
%}

%ignore CauldronIOException;
%rename ("Equals") CauldronIO::Property::operator==(const Property& other) const;
%rename ("Equals") CauldronIO::Reservoir::operator==(const Reservoir& other) const;
%rename ("Equals") CauldronIO::Formation::operator==(const Formation& other) const;
%rename ("Equals") CauldronIO::Geometry2D::operator==(const Geometry2D& other) const;
%rename ("Equals") CauldronIO::Geometry3D::operator==(const Geometry3D& other) const;
%rename ("Equals") CauldronIO::Project::operator==(const Project& other) const;
%rename ("Equals") CauldronIO::Surface::operator==(const Surface& other) const;
%include "VisualizationAPIFwDecl.h"
%include "VisualizationAPI.h"
%include "VisualizationIO_native.h"
%include "ImportExport.h"

/* some templates */
%template(SnapShotList) std::vector < std::shared_ptr<CauldronIO::SnapShot > >;
%template(FormationVolume) std::pair<std::shared_ptr<const CauldronIO::Formation>, std::shared_ptr<CauldronIO::Volume> >;
%template(FormationVolumeList) std::vector< CauldronIO::FormationVolume >;
%template(SurfaceList) std::vector<std::shared_ptr<CauldronIO::Surface > > ;
%template(VolumeList) std::vector<std::shared_ptr<CauldronIO::Volume > > ;
%template(TrapperList) std::vector<std::shared_ptr<CauldronIO::Trapper > > ;
%template(PropertyList) std::vector<std::shared_ptr<const CauldronIO::Property > > ;
%template(PropertySurfaceData) std::pair<std::shared_ptr<const CauldronIO::Property>, std::shared_ptr<CauldronIO::SurfaceData> >;
%template(PropertySurfaceDataList) std::vector < CauldronIO::PropertySurfaceData > ;
%template(ReservoirList) std::vector<std::shared_ptr<const CauldronIO::Reservoir> >  ;
%template(FormationList) std::vector<std::shared_ptr<const CauldronIO::Formation> > ;
%template(SurfaceNameList) std::vector<std::string> ;
%template(PropertyVolumeData) std::pair<std::shared_ptr<const CauldronIO::Property>, std::shared_ptr<CauldronIO::VolumeData> >;
%template(PropertyVolumeDataList) std::vector<CauldronIO::PropertyVolumeData > ;
%template(HDFinfoList) std::vector<std::shared_ptr<CauldronIO::HDFinfo> >;
%template(GeometryList) std::vector<std::shared_ptr<const CauldronIO::Geometry2D> >;
