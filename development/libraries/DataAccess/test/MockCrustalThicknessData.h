//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MOCK_CRUSTALTHICKNESS_DATA_H
#define MOCK_CRUSTALTHICKNESS_DATA_H

// std library
#include <map>

// DataAccess library
#include "../src/GridMap.h"
#include "../src/CrustalThicknessData.h"

using DataAccess::Interface::SnapshotList;

namespace DataAccess
{
   namespace UnitTests
   {

      /// @class CrustalThicknessData The CTC unit test interface which mimicks data from the CTCIoTbl
      /// @details Use mutators do define data, and accessors to access it
      class MockCrustalThicknessData : public CrustalThicknessData
      {
         public:
            /// @brief Uses nullptr as project handle and record to construct parent objects
            MockCrustalThicknessData() :CrustalThicknessData( nullptr, nullptr ) {}
            virtual ~MockCrustalThicknessData() {}

            /// @defgroup Accessors
            /// @brief Use it to get CTCIoTbl data in your unit tests
            /// @{
               /// @defgroups GeneralInputs
               /// @brief Are defined in the project file via the UI
               /// @{
               /// @brief Return the filter half width, this is the smoothing radius
               virtual int getFilterHalfWidth() const final
               {
                  return m_smoothingRadius;
               }
               /// @brief Create a map for the corresponding attribute if it doesn't exist yet
               virtual Interface::GridMap const * getMap( const Interface::CTCMapAttributeId attributeId ) const final
               {
                  return m_ctcIoTblMaps.find( attributeId )->second;
               }
               /// @brief Return the stratigraphic snapshots
               virtual std::vector<double> getSnapshots() const final
               {
                  return m_snapshots;
               }
               /// @}

               /// @defgroup DebugInputs
               /// @brief Are defined in the project file manually (R&D only)
               /// @{
               /// @brief Return the ratio used to define the lower and upper part of the continental crust (r=upper/low)
               virtual double getUpperLowerContinentalCrustRatio() const final
               {
                  return m_upperLowerContinentalCrustRatio;
               }
               /// @brief Return the ratio used to define the lower and upper part of the oceanic crust (r=upper/low)
               virtual double getUpperLowerOceanicCrustRatio() const final
               {
                  return m_upperLowerOceanicCrustRatio;
               }
               /// @brief Return the name of a base of syn-rift 
               virtual const std::string& getSurfaceName() const final
               {
                  return m_surfaceName;
               }
               /// @}
            /// @}

            /// @defgroup Mutators
            /// @brief Use it to set CTCIoTbl data in your unit tests
            /// @{
               /// @defgroups GeneralInputs
               /// @brief Are defined in the project file via the UI
               /// @{
               /// @brief Return the filter half width, this is the smoothing radius
               void setFilterHalfWidth( const int value )
               {
                  m_smoothingRadius = value;
               }
               void setMap( const Interface::CTCMapAttributeId attributeId, GridMap const * map )
               {
                  m_ctcIoTblMaps[attributeId] = map;
               }
               /// @brief Set the stratigraphic snapshots
               void setSnapshots( const std::vector<double>& list )
               {
                  m_snapshots = list;
               }
               /// @}

               /// @defgroup DebugInputs
               /// @brief Are defined in the project file manually (R&D only)
               /// @{
               void setUpperLowerContinentalCrustRatio( const double value )
               {
                  m_upperLowerContinentalCrustRatio = value;
               }
               void setUpperLowerOceanicCrustRatio( const double value )
               {
                  m_upperLowerOceanicCrustRatio = value;
               }
               /// @brief Sets the name of a base of syn-rift 
               void setSurfaceName( const std::string& name )
               {
                  m_surfaceName = name;
               }
               /// @}
            /// @}

         private:
            int m_smoothingRadius;                    ///< The filter half width
            double m_upperLowerContinentalCrustRatio; ///< The upper/lower continental crust ratio
            double m_upperLowerOceanicCrustRatio;     ///< The upper/lower oceanic crust ratio
            std::string m_surfaceName;                ///< The base of the syn-rift
            std::vector<double> m_snapshots;          ///< The list of major stratigraphic snapshots
            /// Stores the input maps (initial continental crust thickness and initial lithospheric mantle thickness)
            std::map< const Interface::CTCMapAttributeId, Interface::GridMap const * > m_ctcIoTblMaps;

      };
   }
}


#endif // MOCK_CRUSTALTHICKNESS_DATA_H
