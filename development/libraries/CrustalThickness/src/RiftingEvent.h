//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CRUSTALTHICKNESS_RIFTINGEVENT_H
#define CRUSTALTHICKNESS_RIFTINGEVENT_H

// DataAccess library
#include "Interface.h"

// forward declarations
namespace DataAccess
{
   namespace Interface
   {
      class GridMap;
   }
}
using DataAccess::Interface::GridMap;
using DataAccess::Interface::TectonicFlag;

namespace CrustalThickness{
   /// @class RiftingEvent A rifting event which corresponds to a line of the rifting history table in the CTC user interface
   class RiftingEvent {

   public:

      /// @throw std::invalid_argument if the seaLevelAdjustment or maximumOceanicCrustThickness maps are nullptr
      RiftingEvent( const TectonicFlag tectonicFlag,
         GridMap const * const seaLevelAdjustment,
         GridMap const * const maximumOceanicCrustThickness );
      ~RiftingEvent() {}

      /// @defgroup Accessors
      /// @{
      TectonicFlag getTectonicFlag()                    const{ return m_tectonicFlag;                 }
      GridMap const * getMaximumOceanicCrustThickness() const{ return m_maximumOceanicCrustThickness; }
      GridMap const * getSeaLevelAdjustment()           const{ return m_seaLevelAdjustment;           }
      double getStartRiftAge () const { return m_startRiftAge;    }
      double getEndRiftAge   () const { return m_endRiftAge;      }
      bool getCalculationMask() const { return m_calculationMask; }
      unsigned int getRiftId () const { return m_riftId;          }
      /// @}

      /// @defgroup Mutators
      /// @{
      void setStartRiftAge   ( const double age      );
      void setEndRiftAge     ( const double age      );
      void setCalculationMask( const bool mask       ) { m_calculationMask = mask; }
      void setRiftId         ( const unsigned int id ) { m_riftId          = id;   }
      /// @}

   private:

      const TectonicFlag m_tectonicFlag;                    ///< The tectonic contexct (active, passive or flexural)
      GridMap const * const m_seaLevelAdjustment;           ///< The sea level adjustement (represents the dynanmic topography) [m]
      GridMap const * const m_maximumOceanicCrustThickness; ///< The maximum oceanic crustal thickness                          [m]
      double m_startRiftAge;                                ///< The begging of the rift                                        [Ma]
      double m_endRiftAge;                                  ///< The end of the rift                                            [Ma]
      bool m_calculationMask;                               ///< If and only if true, then all ctc crust properties are computed and output for this rifting event
      unsigned int m_riftId;                                ///< The rift ID to which belongs the rift event, one rift event belongs to only one rift and vice versa

   };
}
#endif

