#include "../src/PTDiagramCalculator.h"
#include "../src/EosPackCAPI.h"
#include "PVTCfgFileMgr.h"

#include <vector>
#include <sstream>

#include <cstring>
#include <cmath>
#include <cstdio>

#include <omp.h>

#include <gtest/gtest.h>

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

static const    int g_NumOfThreads = 5;

// Test of PTDiagramCalculator in multithred environments
class PTDiagramCalculatorMTTest: public ::testing::Test
{
public:
   PTDiagramCalculatorMTTest()
   {
      PVTCfgFileMgr::instance( "./PVT_properties_DCMT.cfg" ); // Write configuration file
   }

   ~PTDiagramCalculatorMTTest() {;}
};

TEST_F( PTDiagramCalculatorMTTest, CAPI )
{
   std::ostringstream msgs;
   bool flag = true;

   // Start an OpenMP thread pool
   #pragma omp parallel num_threads(g_NumOfThreads) shared(msgs,flag)
   {
      std::ostringstream ofs; // error messages keeper for each thread
      int tid = omp_get_thread_num();

      // composition is created as class member array in m_Composition
      // allocate space for special points
      double points[8]; // 4 points - Critical point, Bubble point, Cricondentherm point, Cricondenbar point

      // allocate space for isolines. It will be 11 isolines 0 : 0.1 : 1, up to 400 points (2 double per point) per isoline
      int szIso[] = { 400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 400 };

      double isolines[11 * 400 * 2];

      // Call diagram builder
      bool ret = pvtFlash::BuildPTDiagram(0, TrapCond[0], TrapCond[1], const_cast<double *>(Composition), points, szIso, isolines);

      // check results
      if ( !ret ) ofs << "PTDiagramBuilder failed to create diagram for unknown reason for thread: " << tid << "\n";

      if ( ret )
      {
         // check isoline sizes
         for ( int i = 0; i < 11; ++i )
         {
            if ( IsolinesSizes[i] != szIso[i] )
            {
               ofs << "Isoline " << i << " lenght " << szIso[i] << " does not match to " << IsolinesSizes[i] << " for thread " << tid << "\n";
               ret = false;
            }
         }

         // check critical points
         if ( std::abs( points[0] - CritPoint[0] ) > 1e-3 )
         {
            ofs << "Wrong critical point temperature value for thread " << tid << "\n";
            ret = false;
         }

         if ( std::abs( points[1] - CritPoint[1] ) > 1e-3 )
         {
            ofs << "Wrong critical point pressure value for thread " << tid << "\n";
            ret = false;
         }

         if ( std::abs( points[2] - BubblePoint[0] ) > 1e-3 )
         {
            ofs << "Wrong bubble point temperature value for thread " << tid << "\n";
            ret = false;
         }

         if ( std::abs( points[3] - BubblePoint[1] ) > 1e-3 )
         {
            ofs << "Wrong bubble point pressure value for thread " << tid << "\n";
            ret = false;
         }

         // search critical point
         for ( int i = 0; i < 8; ++i ) points[i] = 0.0;

         bool ret2 = pvtFlash::FindCriticalPoint( 0, const_cast<double *>(Composition), points );
         if ( !ret2 )
         {
            ofs << "Fast critical point search failed for unknown reason for thread " << tid << "\n";
            ret = false;
         }
         else
         {
            if ( std::abs( points[0] - CritPoint[0] ) > 1e-3 ) 
            {
               ofs << "Wrong critical point temperature value in fast critical point search for thread " << tid << "\n";
               ret = false;
            }
            if ( std::abs( points[1] - CritPoint[1] ) > 1e-3 ) 
            {
               ofs << "Wrong critical point pressure value in fast critical point search for thread " << tid << "\n";
               ret = false;
            }
         }
         #pragma omp critical
         {
            if ( !ret ) // collect return codes for all threads
            {
               msgs << ofs.str();
               flag = false;
            }
         } 
      }
   }
   ASSERT_TRUE( flag ) << msgs.str();
}

TEST_F( PTDiagramCalculatorMTTest, Isolines )
{
   std::ostringstream msgs;
   bool flag = true;

   // Start an OpenMP thread pool
   #pragma omp parallel num_threads(g_NumOfThreads) shared(msgs,flag)
   {
      std::ostringstream ofs; // error messages keeper for each thread
      int tid = omp_get_thread_num();
      bool ret = true;

      std::vector<double> comp(Composition, Composition + sizeof( Composition )/sizeof(double) );

      PTDiagramCalculator diagramBuilder( PTDiagramCalculator::MassFractionDiagram, comp );
      diagramBuilder.setAoverBTerm(2.0);
      diagramBuilder.setNonLinSolverConvPrms( 1e-6, 500, 0.3 );
      diagramBuilder.findBubbleDewLines(TrapCond[0], TrapCond[1], std::vector<double>() );

      for( int i = 0; i < 11; ++i )
      {
         const std::vector< std::pair<double,double> > & isoLine = diagramBuilder.calcContourLine(i * 0.1);

         if ( IsolinesSizes[i] != isoLine.size() )
         {
            ofs << "Isoline " << i << " lenght " << isoLine.size() << " does not match to " << IsolinesSizes[i] << " for thread " << tid << "\n";
            ret = false;
         }
      }

      #pragma omp critical
      {
         if ( !ret ) // collect return codes for all threads
         {
            msgs << ofs.str();
            flag = false;
         }
      }
   }
   ASSERT_TRUE( flag ) << msgs.str(); // final check by gtest
}



