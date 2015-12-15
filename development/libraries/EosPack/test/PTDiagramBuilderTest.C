#include "../src/EosPackCAPI.h"
#include "../src/PTDiagramCalculator.h"
#include "PVTCfgFileMgr.h"

#include <vector>

#include <cstring>
#include <cmath>
#include <cstdio>

#include <gtest/gtest.h>

using namespace pvtFlash;

const double Composition[] = { 4.63937e+10, 3.76229e+11, 7.53617e+10, 1.60934e+11, 7.01346e+10, 1.93474e+11,
                               6.20802e+09, 6.26855e+09, 6.3693e+09,  7.4708e+09, 7.78099e+09, 0, 2.65935e+09,
                               5.4181e+08, 8.16853e+09, 2.68247e+10, 4.82603e+09, 5.65998e+07, 2.17633e+07,
                               0, 0, 0, 0 };

const double CompositionByOlivier[] = {  
      46393676972.0415, // asphaltene
      376228852155.787, // resin
      75361672187.7905, // C15PlusAro
      160933558529.36,  // C15PlusSat
      70134580791.6047, // C6Minus14Aro
      193473629375.084, // C6Minus14Sat 
      6208020563.40107, // C5
      6268552951.99726, // C4
      6369301385.92029, // C3
      7470802663.62787, // C2
      7780993477.8834,  // C1
      0.0,              // COx
      2659345319.13799, // N2
      541810267.726579, // H2S
      8168527698.32282, // LSC
      26824731512.8973, // C15PlusAT
      4826027990.49316, // C6Minus14BT
      56599766.62811,   // C6Minus14DBT
      21763290.0290721, // C6Minus14BP
      0.0,              // C15PlusAroS
      0.0,              // C15PlusSatS
      0.0,              // C6Minus14SatS
      0.0               // C6Minus14AroS
};
 

const double TrapCond[]      = { 588.15, 12665600};
const int    IsolinesSizes[] = { 66, 49, 55, 56, 58, 62, 70, 79, 89, 93, 124 };
const double CritPoint[]     = { 909.17673, 10959091.306859 };
const double BubblePoint[]   = { 588.15, 10057971.0253 };
const double eps             = 1.e-3;

class PTDiagramCalculatorTest: public ::testing::Test
{
public:
   PTDiagramCalculatorTest()  { PVTCfgFileMgr::instance(); } // write cfg file
   ~PTDiagramCalculatorTest() {;}
};


TEST_F( PTDiagramCalculatorTest, CAPI )
{
   // composition is created as class member array in m_Composition
   // allocate space for special points
   double points[8]; // 4 points - Critical point, Bubble point, Cricondentherm point, Cricondenbar point

   // allocate space for isolines. It will be 11 isolines 0 : 0.1 : 1, up to 400 points (2 double per point) per isoline
   int szIso[] = { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 };

   double isolines[11 * 400 * 2];

   // Call diagram builder
   bool ret = pvtFlash::BuildPTDiagram(0, TrapCond[0], TrapCond[1], const_cast<double *>(Composition), points, szIso, isolines);

   // check results
   ASSERT_TRUE( ret ); //  PTDiagramBuilder failed to create diagram for unknown reason

   for ( int i = 0; i < 11; ++i )
   {
      EXPECT_EQ( IsolinesSizes[i], szIso[i] );
   }

   EXPECT_NEAR(points[0],  CritPoint[0], 1e-3 ) << "Wrong critical point temperature value";
   EXPECT_NEAR(points[1] , CritPoint[1], 1e-3 ) << "Wrong critical point pressure value";
   EXPECT_NEAR(points[2] , BubblePoint[0], 1e-3)<< "Wrong bubble point temperature value";
   EXPECT_NEAR(points[3] , BubblePoint[1], 1e-3)<< "Wrong bubble point pressure value";

   // search critical point
   for ( int i = 0; i < 8; ++i ) points[i] = 0.0;

   ret = pvtFlash::FindCriticalPoint( 0, const_cast<double *>(Composition), points );   
   ASSERT_TRUE( ret ) << "critical point search shouldn't failed here";
   EXPECT_NEAR( points[0], CritPoint[0], 1e-3 ) << "Wrong critical point temperature value";
   EXPECT_NEAR( points[1], CritPoint[1], 1e-3 ) << "Wrong critical point pressure value";
}

TEST_F( PTDiagramCalculatorTest, BubbleDew)
{
   // create EosPack config file
   std::vector<double> comp(Composition, Composition + sizeof( Composition)/sizeof(Composition[0]) );

   PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MassFractionDiagram, comp );
   diagramBuilder.setAoverBTerm(2.0);
   diagramBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );
   diagramBuilder.findBubbleDewLines(TrapCond[0], TrapCond[1], std::vector<double>() );

   // Check special points
   const std::pair<double,double> & critPt = diagramBuilder.getCriticalPoint();
   EXPECT_NEAR(critPt.first  , CritPoint[0], eps ) << "Wrong critical point temperature value";
   EXPECT_NEAR(critPt.second , CritPoint[1], eps ) << "Wrong critical point pressure value";
   double bubbleP;
   bool ret = diagramBuilder.getBubblePressure( BubblePoint[0], &bubbleP ) ;
   ASSERT_TRUE( ret );
   EXPECT_NEAR(bubbleP , BubblePoint[1], eps ) << "Wrong bubble point pressure value";
   const std::vector< std::pair<double,double> > & dewLine = diagramBuilder.calcContourLine(0.0); // get dew line
   EXPECT_EQ( IsolinesSizes[0], dewLine.size() );

   const std::vector< std::pair<double,double> > & bubbleLine = diagramBuilder.calcContourLine(1.0); // get dew line
   EXPECT_EQ( IsolinesSizes[10], bubbleLine.size() );
}

TEST_F( PTDiagramCalculatorTest, Isolines )
{
   std::vector<double> comp(Composition, Composition + sizeof( Composition )/sizeof(double) );

   PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MassFractionDiagram, comp );
   diagramBuilder.setAoverBTerm(2.0);
   diagramBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );
   diagramBuilder.findBubbleDewLines(TrapCond[0], TrapCond[1], std::vector<double>() );

   for( int i = 0; i < 11; ++i )
   {
      const std::vector< std::pair<double,double> > & isoLine = diagramBuilder.calcContourLine(i * 0.1);
   	EXPECT_EQ( IsolinesSizes[i] , isoLine.size() );
   }
}


TEST_F( PTDiagramCalculatorTest, ObjectAllCountourLinesInOneGo )
{
   std::vector<double> comp(Composition, Composition + sizeof( Composition )/sizeof(double) );

   PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MassFractionDiagram, comp );
   diagramBuilder.setAoverBTerm( 2.0 );
   diagramBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );
   diagramBuilder.findBubbleDewLines(TrapCond[0], TrapCond[1], std::vector<double>() );

   int realSz[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   std::vector<double> values( 11, 0.0 );
   for (int i = 0; i < 11; ++i)
   {
      values[i] = 0.1 * i;
   }

   const std::vector<double> & isoLines = diagramBuilder.calcContourLines(values);

   int isoline = 0;
   for ( int i = 0; i < isoLines.size(); ++i )
   {
      if ( isoLines[i] < 0 && isoLines[i + 1] < 0)
      {
         isoline++;
      }
      else
      {
         realSz[isoline] += 1;
      }
      ++i;
   }

   for (int i = 0; i < 11; ++i)
   {
      EXPECT_EQ( IsolinesSizes[i], realSz[i] );
   }
}

TEST_F( PTDiagramCalculatorTest, TuneAB )
{
   std::vector<double> comp(CompositionByOlivier, CompositionByOlivier + sizeof( CompositionByOlivier )/sizeof(double) );

   PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MoleMassFractionDiagram, comp );
   diagramBuilder.setAoverBTerm( 2.0 );
   diagramBuilder.setNonLinSolverConvPrms( 1e-6, 400, 0.2 );
   double AB = diagramBuilder.searchAoverBTerm();

   EXPECT_NEAR( AB, 3.76929, eps );
}

TEST_F( PTDiagramCalculatorTest, SearchCritPt )
{
   std::vector<double> comp(CompositionByOlivier, CompositionByOlivier + sizeof( CompositionByOlivier )/sizeof(double) );

   PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MoleMassFractionDiagram, comp );
   diagramBuilder.setAoverBTerm( 2.0 );
   diagramBuilder.setNonLinSolverConvPrms( 1e-6, 400, 0.2 );

   const PTDiagramCalculator::TPPoint & critPt = diagramBuilder.searchCriticalPoint();
   double critT = critPt.first;
   double critP = critPt.second;

   EXPECT_NEAR( critT, 903.652, eps );
   EXPECT_NEAR( critP, 11130482.494, eps );
}

