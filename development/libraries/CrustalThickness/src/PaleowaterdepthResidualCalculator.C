//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PaleowaterdepthResidualCalculator.h"
using namespace  CrustalThickness;

// CrustalThicness library
#include "AbstractInterfaceOutput.h"
#include "AbstractValidator.h"
#include "InterfaceInput.h"

// DataAccess library
#include "Interface/Interface.h"

//------------------------------------------------------------//
PaleowaterdepthResidualCalculator::PaleowaterdepthResidualCalculator(
   InterfaceInput& inputData,
   AbstractInterfaceOutput& outputData,
   const DataModel::AbstractValidator& validator,
   const double age,
   const PolyFunction2DArray& surfaceDepthHistory ) :
      m_firstI             ( inputData.firstI() ),
      m_firstJ             ( inputData.firstJ() ),
      m_lastI              ( inputData.lastI() ),
      m_lastJ              ( inputData.lastJ() ),
      m_age                ( age ),
      m_surfaceDepthHistory( surfaceDepthHistory ),
      m_outputData         ( outputData ),
      m_validator          ( validator  )
{}

//------------------------------------------------------------//
void PaleowaterdepthResidualCalculator::compute() const
{
   double PWDR;

   for ( unsigned int i = m_firstI; i <= m_lastI; ++i ) {
      for ( unsigned int j = m_firstJ; j <= m_lastJ; ++j ) {
         const double PWD = m_outputData.getMapValue( CrustalThicknessInterface::outputMaps::isostaticBathymetry, i, j );
         const double surfaceDepthHistory = m_surfaceDepthHistory( i, j ).F( m_age );
         if ( m_validator.isValid( i, j ) and
              PWD != Interface::DefaultUndefinedMapValue and
              surfaceDepthHistory != Interface::DefaultUndefinedMapValue )
         {
              PWDR = calculatePWDR( PWD, surfaceDepthHistory);
         }
         else{
            PWDR = Interface::DefaultUndefinedMapValue;
         }
         m_outputData.setMapValue( CrustalThicknessInterface::outputMaps::PaleowaterdepthResidual, i, j, PWDR );
      }
   }
}

//------------------------------------------------------------//
double PaleowaterdepthResidualCalculator::calculatePWDR( const double  PWD,
                                                         const double  surfaceDepthHistory )
{
   return PWD - surfaceDepthHistory;
}