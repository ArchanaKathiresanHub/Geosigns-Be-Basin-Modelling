/* File : VisualizationAPI.i */
 
%module CauldronIO

%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"
%include "std_pair.i"
%include "typemaps.i"
%include "boost_shared_ptr.i"

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
    %apply unsigned int& OUTPUT { unsigned int& k_range_start };
    %apply unsigned int& OUTPUT { unsigned int& k_range_end };
}

%shared_ptr(CauldronIO::SnapShot)
%shared_ptr(CauldronIO::Surface)
%shared_ptr(CauldronIO::Volume)
%shared_ptr(CauldronIO::DiscontinuousVolume)
%shared_ptr(CauldronIO::Map)
%shared_ptr(CauldronIO::Formation)
%shared_ptr(CauldronIO::Property)
%shared_ptr(CauldronIO::FormationVolume)
%shared_ptr(CauldronIO::Project)
%shared_ptr(CauldronIO::Trapper)
%shared_ptr(CauldronIO::MapNative)
%shared_ptr(CauldronIO::VolumeNative)
%shared_ptr(CauldronIO::DiscontinuousVolumeNative)

%{
/* Includes the header in the wrapper code */
#include VisualizationAPIFwDecl.h
#include "VisualizationAPI.h"
#include "VisualizationIO_native.h"
#include "ImportExport.h"
%}

%ignore CauldronIOException;
%rename ("Equals") CauldronIO::Property::operator==(const Property& other) const;
%include "VisualizationAPIFwDecl.h"
%include "VisualizationAPI.h"
%include "VisualizationIO_native.h"
%include "ImportExport.h"

/* some templates */
%template(SnapShotList) std::vector < boost::shared_ptr<CauldronIO::SnapShot > >;
%template(FormationVolume) std::pair<boost::shared_ptr<CauldronIO::Formation>, boost::shared_ptr<CauldronIO::Volume> >;
%template(FormationVolumeList) std::vector<boost::shared_ptr<CauldronIO::FormationVolume > >;
%template(SurfaceList) std::vector<boost::shared_ptr<CauldronIO::Surface > > ;
%template(VolumeList) std::vector<boost::shared_ptr<CauldronIO::Volume > > ;
%template(DiscontinuousVolumeList) std::vector<boost::shared_ptr<CauldronIO::DiscontinuousVolume > > ;
%template(TrapperList) std::vector<boost::shared_ptr<CauldronIO::Trapper > > ;
%template(PropertyList) std::vector<boost::shared_ptr<const CauldronIO::Property > > ;
