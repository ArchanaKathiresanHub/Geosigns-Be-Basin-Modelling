//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_MOCKINTERFACEINPUT_H
#define CRUSTALTHICKNESS_MOCKINTERFACEINPUT_H

// std library
#include <memory>
#include <vector>

//Parent
#include "../src/InterfaceInput.h"

// CrustalThickness library
#include "../src/ConfigFileParameterCtc.h"
#include "../src/RiftingEvent.h"

// DataAccess library
#include "Interface/GridMap.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/CrustalThicknessRiftingHistoryData.h"

using DataAccess::Interface::CrustalThicknessData;
using DataAccess::Interface::CrustalThicknessRiftingHistoryData;

typedef const std::shared_ptr < const CrustalThicknessData > ctcData;
typedef const std::vector< std::shared_ptr<const CrustalThicknessRiftingHistoryData> > ctcRiftingHistoryData;

typedef std::map<const double, std::shared_ptr<CrustalThickness::RiftingEvent>> riftingEvents;

/// @class MockInterfaceInput Class which sets input data for fastctc unit tests
///    This class should be used as a test feature for google tests
class MockInterfaceInput : public InterfaceInput {

   public:

      /// @brief Use this constructor to test the calculators more easily by setting inputs directly in the interface
      /// @details The constructors specifies null pointers as the project handle and record, so only the functions which are not
      ///    depending on them can be tested with this class
      MockInterfaceInput()
         :InterfaceInput
         (
            ctcData( new CrustalThicknessData( nullptr, nullptr ) ),
            ctcRiftingHistoryData( 1, std::shared_ptr<const CrustalThicknessRiftingHistoryData>( new CrustalThicknessRiftingHistoryData( nullptr, nullptr ) ) )
         ) {};

      /// @brief Use this constructor to test the calculators by setting inputs in the interface via the data objects
      MockInterfaceInput( const std::shared_ptr< const CrustalThicknessData>           crustalThicknessData,
         const std::vector<std::shared_ptr<const CrustalThicknessRiftingHistoryData>>& crustalThicknessRiftingHistoryData )
         :InterfaceInput( crustalThicknessData, crustalThicknessRiftingHistoryData ) {};


      virtual ~MockInterfaceInput() {};

      /// @defgroup Mutators
      /// @brief Use it to quickly set input data in your unit tests
      /// @{
      void setSmoothRadius         ( const unsigned int smoothRadius    ) { m_smoothRadius           = smoothRadius;          }
      void setFlexuralAge          ( const double age                   ) { m_flexuralAge            = age;                   }
      void setContinentalCrustRatio( const double continentalCrustRatio ) { m_continentalCrustRatio  = continentalCrustRatio; }
      void setOceanicCrustRatio    ( const double oceanicCrustRatio     ) { m_oceanicCrustRatio      = oceanicCrustRatio;     }

      void setConstants( const CrustalThickness::ConfigFileParameterCtc& constants ) { m_constants = constants; }

      void setBaseRiftSurfaceName( const string baseRiftName ) { m_baseRiftSurfaceName = baseRiftName; }

      void setHCuMap      (const GridMap* map  ) { m_HCuMap  = map;          }
      void setHLMuMap     (const GridMap* map  ) { m_HLMuMap = map;          }
      void setRiftingEvent(riftingEvents events) { m_riftingEvents = events; }

      void setPressureBasement            ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureBasement           = pressure; }
      void setPressureBasementAtPresentDay( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureBasementPresentDay = pressure; }
      void setPressureWaterBottom         ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureWaterBottom        = pressure; }
      void setPressureMantle              ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureMantle             = pressure; }
      void setPressureMantleAtPresentDay  ( const DerivedProperties::SurfacePropertyPtr pressure) { m_pressureMantleAtPresentDay = pressure; }
      void setDepthBasement               ( const DerivedProperties::SurfacePropertyPtr depth   ) { m_depthBasement              = depth;    }
      void setDepthWaterBottom            ( const DerivedProperties::SurfacePropertyPtr depth   ) { m_depthWaterBottom           = depth;    }

      void setTopOfSedimentSurface(const DataAccess::Interface::Surface* surface) { m_topOfSedimentSurface    = surface; }
      void setBotOfSedimentSurface(const DataAccess::Interface::Surface* surface) { m_bottomOfSedimentSurface = surface; }

      void setSurfaceDepthHistoryMask( std::map< const double, bool>& mask ) { m_hasSurfaceDepthHistory = mask; };

      /// @}


};

#endif