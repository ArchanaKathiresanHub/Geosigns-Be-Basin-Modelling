//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// CrustalThickness library test utilities
#include "McKenzieTester.h"
#include "../src/RiftingEvent.h"

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

using CrustalThickness::UnitTests::McKenzieTester;

McKenzieTester::McKenzieTester() :
   m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 2 ), m_lastJ( 2 ),
   m_minI( 0.0 ), m_minJ( 0.0 ), m_maxI( 4.0 ), m_maxJ( 4.0 ),
   m_numI( 3 ), m_numJ( 3 ),
   m_age( 10.0 ), m_startAge( 100 ), m_endAge( 60 ),
   m_contRatio(0.5), m_oceaRatio( 0.7 ),
   m_constants(),
   m_inputData(),
   m_outputData( this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ ),
   m_validator()
{
   m_grid    = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
   //   - the sea level adjustement (not used in McKenzie's computations but needed to construct rifting events)
   m_seaLevelAdjustment = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 40000, 1 );
   //   - basement depth is 5000m
   m_depthBasement = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ,
      this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "depthBasement", "Depth", 10, 5000 );
   //   - initial continental crustal thickness is 40Km
   m_HCuMap  = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 40000,  1 );
   //   - maximum oceanic crustal thickness is 4Km
   m_HBuMap  = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 4000,   1 );
   //   - initial lithospheric mantle thickness is 115Km
   m_HLMuMap = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 115000, 1 );
   //   - stacked ITS for the rift is 1Km
   m_previousRiftITS = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 1000, 1 );
   //   - previous continental crust thickness is 40Km
   m_previousContinentalCrustThickness = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 40000, 1 );
   //   - previous oceanic crust thickness crust is 200m
   m_previousOceanicCrustThickness     = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 200,   1 );

}

McKenzieTester::~McKenzieTester()
{
   delete m_grid;
   delete m_seaLevelAdjustment;
   delete m_depthBasement;
   delete m_HCuMap;
   delete m_HBuMap;
   delete m_HLMuMap;
   delete m_previousContinentalCrustThickness;
   delete m_previousOceanicCrustThickness;
}

void McKenzieTester::initTestData(){
   m_outputData.clear();
   initConstants( m_constants );
   m_riftingEvents[m_age] = std::shared_ptr<RiftingEvent>( new RiftingEvent( DataAccess::Interface::PASSIVE_MARGIN, m_seaLevelAdjustment, m_HBuMap ) );
   (m_riftingEvents[m_age])->setStartRiftAge( m_startAge );
   (m_riftingEvents[m_age])->setEndRiftAge  ( m_endAge   );
   m_inputData.setRiftingEvent( m_riftingEvents );
   m_inputData.setConstants( m_constants );
   m_inputData.setHCuMap ( m_HCuMap  );
   m_inputData.setHLMuMap( m_HLMuMap );
   m_inputData.setDepthBasement( m_depthBasement->getMockderivedSurfacePropertyPtr() );
   m_inputData.setContinentalCrustRatio( m_contRatio );
   m_inputData.setOceanicCrustRatio    ( m_oceaRatio );
}

void McKenzieTester::initConstants( MockConfigFileParameterCtc& constants ) const
{
   constants.setWaterDensity               ( 1030    );
   constants.setBackstrippingMantleDensity ( 3300    );
   constants.setModelTotalLithoThickness   ( 92500   );
   constants.setDecayConstant              ( 10000   );
   constants.setTau                        ( 42.9    );
   constants.setLithoMantleDensity         ( 3360    );
   constants.setCoeffThermExpansion        ( 3.45e-5 );
   constants.setBaseLithosphericTemperature( 1330    );
   constants.setReferenceCrustThickness    ( 35000   );
   constants.setReferenceCrustDensity      ( 2875    );
   constants.setA                          ( 2.9881  );
   constants.setB                          ( 1050    );
   constants.setC                          ( 280     );
   constants.setD                          ( -294000 );
   constants.setE                          ( 2830    );
   constants.setF                          ( 2840    );
   constants.setInitialSubsidenceMax       ( 4055    );
   constants.setE0                         ( 2656    );
}

const McKenzieCrustCalculatorPtr McKenzieTester::createMcKenzieCalculator()
{
   initTestData();
   return McKenzieCrustCalculatorPtr(
      new McKenzieCrustCalculator( m_inputData,
                                   m_outputData,
                                   m_validator,
                                   m_age,
                                   m_previousRiftITS,
                                   m_previousContinentalCrustThickness,
                                   m_previousOceanicCrustThickness )
                                   );
}
