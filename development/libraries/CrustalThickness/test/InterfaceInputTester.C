//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// CrustalThickness library test utilities
#include "InterfaceInputTester.h"

// utilities library
#include "ConstantsNumerical.h"

// std library
#include <algorithm>

#ifdef WIN32
#include <functional>
#endif // WIN32

using Utilities::Numerical::IbsNoDataValue;
using Utilities::Numerical::UnsignedIntNoDataValue;
using CrustalThickness::UnitTests::InterfaceInputTester;

InterfaceInputTester::InterfaceInputTester() :
   // from present day to oldest age
   m_tectonicFalgs(
      { FLEXURAL_BASIN,
        PASSIVE_MARGIN,
        ACTIVE_RIFTING,
        ACTIVE_RIFTING,
        PASSIVE_MARGIN,
        PASSIVE_MARGIN,
        ACTIVE_RIFTING,
        PASSIVE_MARGIN,
        PASSIVE_MARGIN,
        ACTIVE_RIFTING }
   ),
   m_filterHalfWidth( 10 ),
   m_upperLowerContinentalCrustRatio( 0.5 ),
   m_upperLowerOceanicCrustRatio    ( 0.7 ),
   m_nDVd( IbsNoDataValue ),
   m_nDVi( UnsignedIntNoDataValue ),
   m_grid( 0, 0, 1, 1, 2, 2 ),
   m_HBu    ( m_tectonicFalgs.size(), DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 8000, 1 ) ),
   m_DeltaSL( m_tectonicFalgs.size(), DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 500 , 1 ) ),
   m_projectHandle(new DataAccess::Interface::ProjectHandle(nullptr, "", nullptr)),
   m_ctcGlobalData( std::shared_ptr< MockCrustalThicknessData >( new MockCrustalThicknessData( *m_projectHandle ) ) ),
   m_ctcRiftingDataVec( m_tectonicFalgs.size(), std::shared_ptr< const MockCrustalThicknessRiftingHistoryData >( new MockCrustalThicknessRiftingHistoryData( *m_projectHandle ) ) )
{
   // set default initial continental crust and lithospheric mantle thickness data
   m_HLMuIni = new DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 110000, 1 );
   m_HCuIni  = new DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 40000 , 1 );
   // set default snapshots data
   m_snapshots.push_back( 220.0 );
   m_snapshots.push_back( 200.0 );
   m_snapshots.push_back( 150.0 );
   m_snapshots.push_back( 90.0  );
   m_snapshots.push_back( 80.0  );
   m_snapshots.push_back( 70.0  );
   m_snapshots.push_back( 60.0  );
   m_snapshots.push_back( 50.0  );
   m_snapshots.push_back( 10.0  );
   m_snapshots.push_back( 0.0   );
   // set default surface depth history data
   std::for_each( m_snapshots.begin(), m_snapshots.end(), [&]( const double age ) {
      m_hasSurfaceDepthHistory[age] = true;
   } );
   // set default sea level adjustment data (from present day to oldest age)
   m_DeltaSL[0].setValues( 235     );
   m_DeltaSL[1].setValues( 300     );
   m_DeltaSL[2].setValues( -564.45 );
   m_DeltaSL[3].setValues( 235     );
   m_DeltaSL[4].setValues( 0       );
   m_DeltaSL[5].setValues( 20      );
   m_DeltaSL[6].setValues( 324.45  );
   m_DeltaSL[7].setValues( 0       );
   m_DeltaSL[8].setValues( 0       );
   m_DeltaSL[9].setValues( 18.78   );
   // set default maximum oceanic crust thickness data (from present day to oldest age)
   std::for_each( m_HBu.begin(), m_HBu.end(), []( DataAccess::Interface::SerialGridMap& map ) {
      map.setValues(7000);
   } );
   // check validity of inputs (all the same size)
   assert( m_tectonicFalgs.size()         == m_snapshots.size()
       and m_hasSurfaceDepthHistory.size() == m_snapshots.size()
       and m_DeltaSL.size()               == m_snapshots.size()
       and m_HBu.size()                   == m_snapshots.size() );
}

InterfaceInputTester::~InterfaceInputTester(){
   delete m_HLMuIni;
   delete m_HCuIni;
}

std::shared_ptr<MockInterfaceInput> InterfaceInputTester::createInterfaceInput(){
   updateData();
   std::shared_ptr<MockInterfaceInput> result( new MockInterfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec ) );
   result->loadSnapshots();
   result->setSurfaceDepthHistoryMask( m_hasSurfaceDepthHistory );
   return result;
}

void InterfaceInputTester::updateData(){
   std::sort( m_snapshots.begin(), m_snapshots.end(), std::greater<double>() );
   m_ctcGlobalData->setSnapshots( m_snapshots );
   m_ctcGlobalData->setFilterHalfWidth( m_filterHalfWidth );
   m_ctcGlobalData->setUpperLowerContinentalCrustRatio( m_upperLowerContinentalCrustRatio );
   m_ctcGlobalData->setUpperLowerOceanicCrustRatio    ( m_upperLowerOceanicCrustRatio     );
   m_ctcGlobalData->setMap( DataAccess::Interface::HLMuIni, m_HLMuIni );
   m_ctcGlobalData->setMap( DataAccess::Interface::HCuIni , m_HCuIni );
   m_ctcGlobalData->setSurfaceName( "" );

   m_ctcRiftingDataVec.clear();
   assert( m_HBu.size()     == m_tectonicFalgs.size() );
   assert( m_DeltaSL.size() == m_tectonicFalgs.size() );

   unsigned int i = 0;
   std::for_each( m_tectonicFalgs.begin(), m_tectonicFalgs.end(), [&]( DataAccess::Interface::TectonicFlag flag )
   {
      std::shared_ptr< MockCrustalThicknessRiftingHistoryData > historyData( new MockCrustalThicknessRiftingHistoryData(*m_projectHandle) );
      historyData->setTectonicFlag( flag );
      historyData->setMap( DataAccess::Interface::HBu    , &m_HBu    [i] );
      historyData->setMap( DataAccess::Interface::DeltaSL, &m_DeltaSL[i] );
      m_ctcRiftingDataVec.push_back( historyData );
      i++;
   } );
   assert( m_ctcRiftingDataVec.size() == m_tectonicFalgs.size() );
}

void InterfaceInputTester::EXPECT_RIFT_EQ( const double flexuralAge,
                                           const std::vector< bool         >& expectedCalculationMask,
                                           const std::vector< double       >& expectedStartAge,
                                           const std::vector< double       >& expectedEndAge,
                                           const std::vector< unsigned int >& expectedriftID,
                                           const std::vector< double       >& expectedDeltaSL,
                                           const std::vector< double       >& expectedHBu,
                                           std::shared_ptr<InterfaceInput>    interfaceInput ){

   assert( expectedCalculationMask.size() == m_snapshots.size()
       and expectedStartAge       .size() == m_snapshots.size()
       and expectedEndAge         .size() == m_snapshots.size()
       and expectedDeltaSL        .size() == m_snapshots.size()
       and expectedHBu            .size() == m_snapshots.size() );

   EXPECT_EQ( flexuralAge , interfaceInput->getFlexuralAge()  );

   unsigned int i = 0;
   // from the oldest age to present day
   std::for_each( m_snapshots.begin(), m_snapshots.end(), [&]( const double age ){
      EXPECT_EQ( expectedCalculationMask[i], interfaceInput->getRiftingCalculationMask( age ) );
      EXPECT_EQ( expectedStartAge       [i], interfaceInput->getRiftingStartAge       ( age ) );
      EXPECT_EQ( expectedEndAge         [i], interfaceInput->getRiftingEndAge         ( age ) );
      EXPECT_EQ( expectedriftID         [i], interfaceInput->getRiftId                ( age ) );
      EXPECT_EQ( expectedDeltaSL        [i], interfaceInput->getDeltaSLMap            ( age ).getConstantValue() );
      EXPECT_EQ( expectedHBu            [i], interfaceInput->getHBuMap                ( age ).getConstantValue() );
      i++;
   } );
}
