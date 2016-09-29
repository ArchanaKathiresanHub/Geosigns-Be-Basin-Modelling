//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_MOCKINTERFACEINPUT_H
#define CRUSTALTHICKNESS_MOCKINTERFACEINPUT_H

//Parent
#include "../src/InterfaceInput.h"

// CrustalThickness library
#include "../src/ConfigFileParameterCtc.h"

// DataAccess library
#include "Interface/GridMap.h"

/// @class MockInterfaceInput Class which sets input data for fastctc unit tests
///        Should be used only in unit tests
class MockInterfaceInput : public InterfaceInput {

   public:

      MockInterfaceInput() :InterfaceInput( nullptr, nullptr ){};
      virtual ~MockInterfaceInput() {};

      /// @defgroup Sets
      /// @{
      void setSmoothRadius         ( const unsigned int smoothRadius    ) { m_smoothRadius                    = smoothRadius;          }
      void setFlexuralAge          ( const double t_felxural            ) { m_t_felxural                      = t_felxural;            }
      void setContinentalCrustRatio( const double continentalCrustRatio ) { m_continentalCrustRatio = continentalCrustRatio; }
      void setOceanicCrustRatio( const double oceanicCrustRatio         ) { m_oceanicCrustRatio      = oceanicCrustRatio;    }

      void setConstants( const CrustalThickness::ConfigFileParameterCtc& constants ) { m_constants = constants; }

      void setBaseRiftSurfaceName( const string baseRiftName ) { m_baseRiftSurfaceName = baseRiftName; }

      void  setT0Map     (const GridMap* map) { m_T0Map      = map; }
      void  setTRMap     (const GridMap* map) { m_TRMap      = map; }
      void  setHCuMap    (const GridMap* map) { m_HCuMap     = map; }
      void  setHBuMap    (const GridMap* map) { m_HBuMap     = map; }
      void  setHLMuMap   (const GridMap* map) { m_HLMuMap    = map; }
      void  setDeltaSLMap(const GridMap* map) { m_DeltaSLMap = map; }

      void setPressureBasement          ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureBasement           = pressure; }
      void setPressureWaterBottom       ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureWaterBottom        = pressure; }
      void setPressureMantle            ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureMantle             = pressure; }
      void setPressureMantleAtPresentDay( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureMantleAtPresentDay = pressure; }
      void setDepthBasement             ( const DerivedProperties::SurfacePropertyPtr depth   ) { m_depthBasement              = depth;    }
      void setDepthWaterBottom          ( const DerivedProperties::SurfacePropertyPtr depth   ) { m_depthWaterBottom           = depth;    }

      void setTopOfSedimentSurface(const Interface::Surface* surface) { m_topOfSedimentSurface    = surface; }
      void setBotOfSedimentSurface(const Interface::Surface* surface) { m_bottomOfSedimentSurface = surface; }
      /// @}

};

#endif