//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _CRUSTALTHICKNESS_INTERFACEOUTPUT_H_
#define _CRUSTALTHICKNESS_INTERFACEOUTPUT_H_

// std library
#include <sstream>
#include <string.h>
#include <memory>

// DataAccess library
#include "Interface/CrustalThicknessInterface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/MapWriter.h"
#include "Interface/ObjectFactory.h"
#include "Interface/ProjectHandle.h"

// GeoPhysics library
#include "GeoPhysicsProjectHandle.h"

// CrustalThickness library
#include "AbstractInterfaceOutput.h"
#include "InterfaceInput.h"

// utilities library
#include "FormattingException.h"
#include "LogHandler.h"

using namespace std;
using namespace DataAccess;
using namespace Interface;

using namespace CrustalThicknessInterface;

/// @class InterfaceOutput The CTC output interface
class InterfaceOutput : public AbstractInterfaceOutput {

   typedef formattingexception::GeneralException InterfaceOutputException;

public:
   InterfaceOutput();
   ~InterfaceOutput();

   /// @defgroup SaveMaps
   /// @{
   /// @brief Save output maps to HDF file for the given snapshot
   /// @ param theSnapshot The snapshot at which the output maps are saved
   bool saveOutputMaps            ( ProjectHandle * pHandle, const Snapshot * theSnapshot );
   /// @brief Save output maps to XYZ file
   bool saveXYZOutputMaps         ( ProjectHandle * pHandle );
   /// @brief Save output maps to SUR file
   bool saveExcelSurfaceOutputMaps( ProjectHandle * pHandle );
   /// @brief Save the output maps according to debug mode and output option
   /// @param isDebug True to save debug outputs, false otherwise
   /// @param outputOptions Save as XYZ, SUR, or HDF
   /// @param theSnapshot The snapshot we save
   void saveOutput( ProjectHandle * pHandle, bool isDebug, int outputOptions, const Snapshot * theSnapshot );
   /// @}

   /// @defgroup SetMaps
   /// @{
   /// @brief Set all map's masks to flag
   /// @details If falg is true all the maps will be output, if flag is false nothing will be output
   /// @param flag The flag which sets the map's masks
   void setAllMapsToOutput( const bool flag );
   /// @brief Set the list of maps flag to be true, so this maps will be output
   /// @details This function is an ellipsis. The list of map is 'mapIndex + ...'.
   /// @warning This function must contain numberOfOutputMaps in the ellipsis parameters
   /// @param mapIndex The index of the map
   /// @param ... The indexes of the other maps
   void setMapsToOutput(const outputMaps mapIndex, ... );
   /// @brief Set map's mask to aValue
   /// @param mapIndex The index of the map
   /// @param aValue The value to be set to the map's mask (true by default)
   void setMapToOutput( const outputMaps mapIndex, const bool aValue = true );
   /// @brief Set the value of the map at the node (i,j)
   /// @param mapIndex The index of the map we set the value
   /// @param value The value to set at the node (i,j)
   void setMapValue( const outputMaps mapIndex, const unsigned int i, const unsigned int j, const double value ) override;
   /// @brief Set the values of all the maps at the node (i,j) according to the mask
   void setValuesToMaps( const unsigned int indI, const unsigned int indJ );
   /// @brief Set all map's value to undefined for the node (i,j) according to the mask
   void setAllMapsUndefined( const unsigned int indI, const unsigned int indJ );
   /// @}

   /// @defgroup CreateDeleteMaps
   /// @{
   /// @brief Create all the CTC maps for the defined snapshot
   /// @param[in] theSnapshot The snapshot corresponding to the maps
   /// @param[in] theSurface The stratigraphic surface corresponding to the maps
   /// @param[in] debug If true, then will create debug maps
   void createSnapShotOutputMaps( GeoPhysics::ProjectHandle * pHandle,
                                  std::shared_ptr< const InterfaceInput > interfaceInput,
                                  const Snapshot * theSnapshot,
                                  const Interface::Surface *theSurface,
                                  const bool debug = false );
   /// @brief Create a map for the defined snapshot
   /// @param propertyName The name of the property corresponding to the map
   /// @param theSnapshot The snapshot corresponding to the map
   /// @param theSurface The stratigraphic surface corresponding to the map
   /// @return The map created
   GridMap * createSnapshotResultPropertyValueMap( ProjectHandle * pHandle, const std::string& propertyName, const Snapshot* theSnapshot, 
                                                   const Interface::Surface *theSurface = 0 );
   /// @}

   /// @defgroup DisableOutput
   /// @{
   /// @brief Ensure that the CTC only saves the required maps
   /// @details Some maps are required for the CTC computations but they should be output in debug mode only
   void disableDebugOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot ) const;
   /// @brief Ensure that the CTC only allocates the required maps
   /// @details Some maps can't be created if some inputs are missing
   void updatePossibleOutputsAtSnapshot( const outputMaps id,
                                         const GeoPhysics::ProjectHandle * pHandle,
                                         std::shared_ptr< const InterfaceInput > interfaceInput,
                                         const Snapshot * theSnapshot,
                                         const bool debug );
   /// @}

   /// @defgroup DataUtilities
   /// @{
   /// @brief Retrieve all CTC maps data
   void retrieveData();
   /// @brief Restore all CTC maps data
   void restoreData();
   /// @}

   /// @defgroup MapUtilities
   /// @{
   /// @brief Return the map value
   double & operator []( const outputMaps &mapInd );
   /// @brief Return the map
   GridMap * getMap( const outputMaps &mapInd );
   /// @brief Return the value of the map at node (i,j)
   /// @param mapIndex The index of the map we get the value from
   /// @return The map value
   double getMapValue( outputMaps mapIndex, unsigned int i, unsigned int j ) const;
   /// @brief Return the value of the map mask at node (i,j)
   /// @param mapIndex The index of the map we get the value from
   /// @return The mask value
   bool   getOutputMask( outputMaps mapIndex ) const;
   /// @}

private:
   void disableOutput( ProjectHandle * pHandle, const Interface::Surface* theSurface, const Snapshot* theSnapshot, const std::string& name ) const;

   GridMap * m_outputMaps[numberOfOutputMaps];   ///< List of CTC output maps
   bool m_outputMapsMask [numberOfOutputMaps];   ///< Mask list corresponding to the CTC output maps (true = output, false = no output)
   double m_outputValues [numberOfOutputMaps];   ///< The temporary values of the maps for a defined (i,j) node. Used to set all the map values at once.

   /// @brief Clean all the members of the class
   void clean();
};

//------------------------------------------------------------//

inline void InterfaceOutput::setMapValue(const outputMaps mapIndex, const unsigned int i, const unsigned int j, const double value) 
{

   if( m_outputMaps[mapIndex] != 0 ) {
      m_outputMaps[mapIndex]->setValue( i, j, value );
   }
   // If pointer is null then this is not an error as some maps are debug only
}

//------------------------------------------------------------//

inline double InterfaceOutput::getMapValue(const outputMaps mapIndex, const unsigned int i, const unsigned int j) const
{

   if( m_outputMaps[mapIndex] != 0 ) {
      return m_outputMaps[mapIndex]->getValue( i, j );
   } else {
      throw InterfaceOutputException() << "Map " << CrustalThicknessInterface::outputMapsNames[mapIndex] << " is not allocated.";
   }
}

//------------------------------------------------------------//

inline void InterfaceOutput::setMapToOutput(const outputMaps aMapIndex,const bool aValue) 
{
   m_outputMapsMask[aMapIndex] = aValue;
}

//------------------------------------------------------------//

inline void InterfaceOutput::setValuesToMaps( const unsigned int indI, const unsigned int indJ )
{
   for (int i = 0; i < WLSMap; ++i) {
      if( m_outputMapsMask[i] && m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->setValue( indI, indJ, m_outputValues[i] );
      }
   }
   memset(m_outputValues, 0, numberOfOutputMaps * sizeof(double));
}

//------------------------------------------------------------//

inline void InterfaceOutput::setAllMapsUndefined( const unsigned int indI, const unsigned int indJ ) 
{
   for(int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMapsMask[i]  && m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->setValue( indI, indJ, Interface::DefaultUndefinedMapValue );
      }
   }
   memset(m_outputValues, 0, numberOfOutputMaps * sizeof(double));
}

//------------------------------------------------------------//

inline bool InterfaceOutput::getOutputMask(const outputMaps mapIndex) const
{
   return m_outputMapsMask[mapIndex];
} 

//------------------------------------------------------------//

inline double & InterfaceOutput::operator[]( const outputMaps &mapInd ) 
{
   return m_outputValues[mapInd];
}

//------------------------------------------------------------//

inline GridMap * InterfaceOutput::getMap( const outputMaps &mapInd ) 
{
   return m_outputMaps[mapInd];
}

#endif
