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

#include <vector>

#include <boost/shared_ptr.hpp>

/// @file This file keeps enum data type definitions, because they are can't be forward declared
/// and various forward declarations for VisualizationAPI library
// Forward declarations

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

    class SnapShot;
    class Project;
    class Surface;
    class Map;
    class Formation;
    class Property;
    class Volume;
    class Trapper;
    class DiscontinuousVolume;
    struct FormationInfo;
    typedef std::vector<boost::shared_ptr<FormationInfo> > FormationInfoList;
}


#endif //_VisualizationAPIFwDecl_h_
