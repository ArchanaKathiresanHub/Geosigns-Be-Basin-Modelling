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

using CrustalThickness::UnitTests::InterfaceInputTester;

InterfaceInputTester::InterfaceInputTester() :
   m_tectonicFalgs({ FLEXURAL_BASIN, PASSIVE_MARGIN, ACTIVE_RIFTING, ACTIVE_RIFTING, PASSIVE_MARGIN, PASSIVE_MARGIN }),
   m_filterHalfWidth( 10 ),
   m_upperLowerContinentalCrustRatio( 0.5 ),
   m_upperLowerOceanicCrustRatio    ( 0.7 ),
   m_nDVd( DataAccess::Interface::DefaultUndefinedScalarValue),
   m_nDVi( DataAccess::Interface::DefaultUndefinedMapValueInteger ),
   m_grid( 0, 0, 1, 1, 2, 2 ),
   m_HBu    ( 6, DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 8000, 1 ) ),
   m_DeltaSL( 6, DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 500 , 1 ) ),
   m_ctcGlobalData( std::shared_ptr< MockCrustalThicknessData >( new MockCrustalThicknessData() ) ),
   m_ctcRiftingDataVec( 6, std::shared_ptr< const MockCrustalThicknessRiftingHistoryData >( new MockCrustalThicknessRiftingHistoryData() ) )
{

   m_HLMuIni = new DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 110000, 1 );
   m_HCuIni  = new DataAccess::Interface::SerialGridMap( nullptr, 0, &m_grid, 40000 , 1 );
   m_snapshots.push_back( 0.0    );
   m_snapshots.push_back( 10.0   );
   m_snapshots.push_back( 20.0   );
   m_snapshots.push_back( 50.0   );
   m_snapshots.push_back( 100.0  );
   m_snapshots.push_back( 253.12 );
}

InterfaceInputTester::~InterfaceInputTester(){
   delete m_HLMuIni;
   delete m_HCuIni;
}

std::shared_ptr<InterfaceInput> InterfaceInputTester::createInterfaceInput(){
   updateData();
   return std::shared_ptr<InterfaceInput>( new InterfaceInput( m_ctcGlobalData, m_ctcRiftingDataVec ) );
}


void InterfaceInputTester::updateData(){
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
   m_DeltaSL[0].setValues( 235     );
   m_DeltaSL[1].setValues( 300     );
   m_DeltaSL[2].setValues( -564.45 );
   m_DeltaSL[3].setValues( 235     );
   m_DeltaSL[4].setValues( 0       );
   m_DeltaSL[5].setValues( -562.45 );
   m_HBu[0].setValues( 578  );
   m_HBu[1].setValues( 0    );
   m_HBu[2].setValues( 7468 );
   m_HBu[3].setValues( 5120 );
   m_HBu[4].setValues( 3540 );
   m_HBu[5].setValues( 7468 );

   unsigned int i = 0;
   std::for_each( m_tectonicFalgs.begin(), m_tectonicFalgs.end(), [&]( DataAccess::Interface::TectonicFlag flag )
   {
      std::shared_ptr< MockCrustalThicknessRiftingHistoryData > historyData( new MockCrustalThicknessRiftingHistoryData() );
      historyData->setTectonicFlag( flag );
      historyData->setMap( DataAccess::Interface::HBu    , &m_HBu    [i] );
      historyData->setMap( DataAccess::Interface::DeltaSL, &m_DeltaSL[i] );
      m_ctcRiftingDataVec.push_back( historyData );
      i++;
   } );
   assert( m_ctcRiftingDataVec.size() == m_tectonicFalgs.size() );
}

void InterfaceInputTester::EXPECT_RIFT_EQ( const double firstRiftAge,
                                           const double flexuralAge,
                                           const std::vector< double       >& expectedStartAge,
                                           const std::vector< double       >& expectedEndAge,
                                           const std::vector< unsigned int >& expectedriftID,
                                           const std::vector< double       >& expectedDeltaSL,
                                           const std::vector< double       >& expectedHBu,
                                           std::shared_ptr<InterfaceInput> interfaceInput ){

   assert( expectedStartAge.size() == m_snapshots.size()
      and expectedEndAge   .size() == m_snapshots.size()
      and expectedDeltaSL  .size() == m_snapshots.size()
      and expectedHBu      .size() == m_snapshots.size() );

   EXPECT_EQ( firstRiftAge, interfaceInput->getFirstRiftAge() );
   EXPECT_EQ( flexuralAge , interfaceInput->getFlexuralAge()  );

   unsigned int i = 0;
   std::for_each( m_snapshots.begin(), m_snapshots.end(), [&]( const double age ){
      EXPECT_EQ( expectedStartAge[i], interfaceInput->getRiftingStartAge( age ) );
      EXPECT_EQ( expectedEndAge  [i], interfaceInput->getRiftingEndAge  ( age ) );
      EXPECT_EQ( expectedriftID  [i], interfaceInput->getRiftId         ( age ) );
      EXPECT_EQ( expectedDeltaSL [i], interfaceInput->getDeltaSLMap     ( age ).getConstantValue() );
      EXPECT_EQ( expectedHBu     [i], interfaceInput->getHBuMap         ( age ).getConstantValue() );
      i++;
   } );
}
