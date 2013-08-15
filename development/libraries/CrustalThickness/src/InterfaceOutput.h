#ifndef __Interface_OUTPUT_H__
#define __Interface_OUTPUT_H__

#include <sstream>
#include <string.h>

#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "Interface/MapWriter.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ObjectFactory.h"

#include "InterfaceDefs.h"

using namespace std;
using namespace DataAccess;
using namespace Interface;


using namespace CrustalThicknessInterface;

class InterfaceOutput {

public:
   InterfaceOutput();

   ~InterfaceOutput();

   bool allocateOutputMaps( ProjectHandle * pHandle );
   void allocateOutputMap ( ProjectHandle * pHandle, outputMaps aMapIndex );
   bool saveOutputMaps( ProjectHandle * pHandle, const Snapshot * theSnapshot );
   bool saveXYZOutputMaps( ProjectHandle * pHandle );
   bool saveExcelSurfaceOutputMaps( ProjectHandle * pHandle );

   void setAllMapsToOutput( bool flag );
   void setMapsToOutput( outputMaps mapIndex, ... );
   void setMapToOutput( outputMaps mapIndex, bool aValue = true );
   void setMapValue( outputMaps mapIndex, unsigned int i, unsigned int j, const double value ); 
   // set values to output maps according to mask
   void setValuesToMaps( unsigned int indI, unsigned int indJ );
   
   void setAllMapsUndefined( unsigned int indI, unsigned int indJ );   
   bool updateIsoBathymetryMaps ( ProjectHandle * pHandle, std::vector<double> &snapshotsList );
   GridMap * getCurrentPropertyMap( ProjectHandle * pHandle, double aSnapshotId, const std::string& propertyName );

   double getMapValue( outputMaps mapIndex, unsigned int i, unsigned int j ) const; 
   bool   getOutputMask( outputMaps mapIndex ) const; 
   void   deleteOutputMap( outputMaps mapIndex );
   bool   createSnapShotOutputMaps( ProjectHandle * pHandle, const Snapshot* theSnapshot, const Interface::Surface *theSurface  = 0 );

   GridMap * createSnapshotResultPropertyValueMap ( ProjectHandle * pHandle, const std::string& propertyName, const Snapshot* theSnapshot, 
                                                    const Interface::Surface *theSurface = 0 );
 
   void retrieveData();
   void restoreData();

   // Return the map value
   double & operator []( const outputMaps &mapInd );
   // Return the map
   GridMap * getMap( const outputMaps &mapInd );

   void debugOutput( ProjectHandle * pHandle, bool isDebug, int outputOptions, const Snapshot * theSnapshot );
private:

   GridMap * m_outputMaps[numberOfOutputMaps];
   bool m_outputMapsMask[numberOfOutputMaps];
   double m_outputValues[numberOfOutputMaps];

   void clean();
};

inline void InterfaceOutput::setMapValue(outputMaps mapIndex, unsigned int i, unsigned int j, const double value) 
{

   if( m_outputMaps[mapIndex] != 0 ) {
      m_outputMaps[mapIndex]->setValue( i, j, value );
   } else {
      // stringstream ss;
      // ss << "Map " << CrustalThicknessInterface::outputMapsNames[mapIndex] << " is not allocated." << endl;
      // throw ss.str();
   }
}

inline double InterfaceOutput::getMapValue(outputMaps mapIndex, unsigned int i, unsigned int j) const
{

   if( m_outputMaps[mapIndex] != 0 ) {
      return m_outputMaps[mapIndex]->getValue( i, j );
   } else {
      return  Interface::DefaultUndefinedMapValue;
      // stringstream ss;
      // ss << "Map " << CrustalThicknessInterface::outputMapsNames[mapIndex] << " is not allocated." << endl;
      // throw ss.str();
   }
}
inline void InterfaceOutput::setMapToOutput(outputMaps aMapIndex, bool aValue) 
{
   m_outputMapsMask[aMapIndex] = aValue;
}

inline void InterfaceOutput::setValuesToMaps( unsigned int indI, unsigned int indJ )
{
   for(int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMapsMask[i] && m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->setValue( indI, indJ, m_outputValues[i] );
      }
   }
   memset(m_outputValues, 0, numberOfOutputMaps * sizeof(double));
}

inline void InterfaceOutput::setAllMapsUndefined( unsigned int indI, unsigned int indJ ) 
{
   for(int i = 0; i < numberOfOutputMaps; ++ i ) {
      if( m_outputMapsMask[i]  && m_outputMaps[i] != 0 ) {
         m_outputMaps[i]->setValue( indI, indJ, Interface::DefaultUndefinedMapValue );
      }
   }
   memset(m_outputValues, 0, numberOfOutputMaps * sizeof(double));
}
inline bool InterfaceOutput::getOutputMask(outputMaps mapIndex) const
{
   return m_outputMapsMask[mapIndex];
} 

inline double & InterfaceOutput::operator[]( const outputMaps &mapInd ) 
{
   return m_outputValues[mapInd];
}

inline GridMap * InterfaceOutput::getMap( const outputMaps &mapInd ) 
{
   return m_outputMaps[mapInd];
}

#endif  __Interface_OUTPUT_H__
