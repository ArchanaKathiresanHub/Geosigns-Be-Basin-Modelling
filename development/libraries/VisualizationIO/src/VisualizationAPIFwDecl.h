//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _VisualizationAPIFwDecl_h_
#define _VisualizationAPIFwDecl_h_

#define xml_version_major 0
                             // version 1: initial version
#define xml_version_minor 2  // version 2: changed geometry storage

#include <vector>
#include <memory>

/// This file keeps enum data type definitions, because they are can't be forward declared
/// and various forward declarations for VisualizationAPI library
/// Forward declarations

namespace DataAccess
{
    namespace Interface
    {
        class Formation;
        class Snapshot;
        class ProjectHandle;
        class PropertyValue;
        class Surface;
        class Formation;
        class Property;
        typedef std::vector<const PropertyValue *> PropertyValueList;
        typedef std::vector<const Property *> PropertyList;
    }
}

namespace CauldronIO
{
    // enums

    /// \enum ModellingMode
    /// \brief FastCauldron modeling modes
    enum ModellingMode { MODE1D, MODE3D };
    /// \enum SnapShotKind 
    /// \brief Type of snapshot
    enum SnapShotKind { SYSTEM, USERDEFINED, NONE };
    /// \enum PropertyAttribute
    /// \brief Type of property
    enum PropertyAttribute { Continuous3DProperty, Discontinuous3DProperty, Surface2DProperty, Formation2DProperty, Other};
    /// \enum PropertyType
    /// \brief Type of property
    enum PropertyType { ReservoirProperty, FormationProperty, TrapProperty };
    /// \enum SubsurfaceKind
    /// \brief Type of subsurface
    enum SubsurfaceKind { Sediment, Basement, None };
    /// \enum SpeciesNamesId
    /// \brief The hydrocarbone species
    enum SpeciesNamesId  { FIRST_COMPONENT = 0, ASPHALTENE = 0, RESIN = 1, C15_PLUS_ARO = 2, C15_PLUS_SAT = 3, C6_MINUS_14ARO = 4, C6_MINUS_14SAT = 5,
                           C5 = 6, C4 = 7, C3 = 8, C2 = 9, C1 = 10, COX = 11, N2 = 12, H2S = 13, LSC = 14, C15_PLUS_AT = 15, C6_MINUS_14BT = 16,
                           C6_MINUS_14DBT = 17, C6_MINUS_14BP = 18, C15_PLUS_ARO_S = 19, C15_PLUS_SAT_S = 20, C6_MINUS_14SAT_S = 21, C6_MINUS_14ARO_S = 22,
                           NUMBER_OF_SPECIES = 23 };

    /// \enum FormationMapType
    enum FormationMapType {FIRSTMAP = 0, THICKNESS = 0, SRMIXINGHI = 1, LITHOTYPE1 = 2,  LITHOTYPE2 = 3, LITHOTYPE3 = 4, LASTMAP = 5}; 

    class SnapShot;
    class Project;
    class Surface;
    class SurfaceData;
    class Formation;
    class Property;
    class Volume;
    class VolumeData;
    class Geometry2D;
    class Geometry3D;
    class Trapper;
    class Trap;
    class Reservoir;
    class VisualizationIOData;
    class StratigraphyTableEntry;
    class MigrationEvent;
    class DisplayContour;
    class IsoEntry;
    class Biomarkerm;
    class FtSample;
    class FtGrain;
    class FtPredLengthCountsHist;
    class FtPredLengthCountsHistData;
    class FtClWeightPercBins;
    class SmectiteIllite;
    class DepthIo;
    class TimeIo1D;
    struct FormationInfo;
    typedef std::vector<std::shared_ptr<FormationInfo> > FormationInfoList;
}


#endif //_VisualizationAPIFwDecl_h_
