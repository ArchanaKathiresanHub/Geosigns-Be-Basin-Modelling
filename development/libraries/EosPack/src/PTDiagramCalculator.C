//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "PTDiagramCalculator.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include <memory>
#include <numeric>
#include <set>
#include <algorithm>
#include <stack>

/// \brief Definition of composition phases as enum, the main idea of this enum that liquidPhase + vaporPhase == bothPhases
///        this used in bisectioning algorithms, also unknown is used as sign of uninitialised data or in return if something failed
typedef enum 
{  unknown     = 0,  ///!< in case of failing algorithms used as return value
   liquidPhase = 1,  ///!< composition has liquid phase
   vaporPhase  = 2,  ///!< composition has vapor phase
   bothPhases  = 3   ///!< composition has both phases
} CompositionPhase;

// The default diagram settings
static const double g_MinimalTemperature = 173.15;   // -100 C
static const double g_MaximalTemperature = 1123;     // +850 C
static const double g_MinimalPressure    = 101325.0; // stock tank pressure - 1 atm
static const double g_MaximalPressure    = 30e6;     // default maximum Pressure value for diagram 30 MPa
static const double g_MaximalDiagramPressure = g_MaximalPressure * 5; // absolute maximum pressure for diagram. Sometimes it could be more than 30 MPa 
static const int    g_GridSize           = 100;      // number of points between Min/Max temperature and pressure
static double       g_Tolerance          = 1e-4;     // stop tolerance for bisection iterations
static const int    g_MaxStepsNum        = 200;      // maximum steps number in bisections iterations
static const int    g_GridExtStep        = 5;        // extend T grid when needed for this number of points
static const double g_DefaultAoverB      = 2.0;      // the default value for A/B term
static const int    g_maxLGRLevel        = 3;        // max level of cell refinement during tracing isoline near bubble/dew line 

inline bool Between( double val, double x1, double x2 ) { return val > std::min( x1, x2 ) && val < std::max( x1, x2 ); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Constructor. It also creates default grid for T from g_MinimalTemperature:g_MinimalTemperature
// typeOfDiagram which type of diagram should be built - Mass/Mole/Volume fraction
// massFraction array of component masses
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::PTDiagramCalculator( DiagramType typeOfDiagram, const std::vector<double> & massFraction ) : 
      m_eps( g_Tolerance ),
      m_critP( -1.0 ),
      m_critT( -1.0 ),
      m_bdBisecIters( 0 ),
      m_isoBisecIters( 0 ),
      m_abTuneBisecIters( 0 ),
      m_AoverB( g_DefaultAoverB ),
      m_ChangeAoverB( false ),
      m_newtonRelCoeff(1.0),
      m_stopTol( g_Tolerance * 1e-2 ),
      m_maxIters( g_MaxStepsNum * 2 )
{
   m_flasher.reset( pvtFlash::EosPack::createNewInstance() );
   m_diagType = typeOfDiagram;
   m_masses   = massFraction;

   assert( m_masses.size() == ComponentId::NUMBER_OF_SPECIES );

   // create default grid for temperature and pressure
   generatePTGrid( g_MinimalPressure, g_MaximalPressure, g_MinimalTemperature, g_MaximalTemperature );

   m_epsT = m_eps * (m_gridT.back() - m_gridT.front())/static_cast<double>(m_gridT.size()); // epsT = eps * dT
   m_epsP = m_eps * (m_gridP.back() - m_gridP.front())/static_cast<double>(m_gridP.size()); // epsP = eps * dP
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generate P and T 1D grids, initialise 2D array for liquid fraction
///////////////////////////////////////////////////////////////////////////////////////////////////
void PTDiagramCalculator::generatePTGrid( double minP, double maxP, double minT, double maxT )
{
   m_gridT.resize( g_GridSize, minT );
   m_gridP.resize( g_GridSize, minP );

   // create T grid using weak geometry progression to get more points around critical point
   double s = (maxT - minT) / static_cast<double>( m_gridT.size() - 1 ); 
   for ( size_t i = 1; i < m_gridT.size(); ++i )
   {
      m_gridT[i] = m_gridT[i-1] + s;
   }

   m_liqFrac.clear();
   for ( size_t i = 0; i < m_gridP.size(); ++i )
   {
      // init 2D liquid fraction array
      m_liqFrac.push_back( std::vector<double>( m_gridT.size(), -1.0 ) );
      m_gridP[i] = minP + i * ( maxP - minP ) / ( m_gridP.size() - 1 );
   }
   
   // check that on uper bound there is no 2 phase region
   bool maxFound = false;
   while( !maxFound && m_gridP.back() < (g_MaximalDiagramPressure - g_MaximalPressure/2)  )
   {
      maxFound = true;
      for ( size_t i = 0; i < m_gridT.size(); i += m_gridT.size() / 10 )
      {
         double phaseFrac[2];
         int phase = getMassFractions( m_gridP[m_gridP.size() - 1], m_gridT[i], m_masses, phaseFrac );
         ++m_bdBisecIters;

         if ( bothPhases == phase ) // two phase region
         {
            maxFound = false;
            break;
         }
      }
      if ( !maxFound )
      {
         extendPGrid( m_gridP.back() + 10e6 );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Extend P grid to a new maximal given value. Also resize 2D liquid fraction arrray
// newPMax new maximal value for Pressure
///////////////////////////////////////////////////////////////////////////////////////////////////
void PTDiagramCalculator::extendPGrid( double newPMax )
{
   if ( newPMax < m_gridP.back() ) return;

   size_t oldSize  = m_gridP.size();
   double presStep = m_gridP[oldSize - 1] - m_gridP[oldSize - 2];
   size_t newSize  = oldSize + static_cast<size_t>(std::floor( ( newPMax - m_gridP.back() ) / presStep + 0.5 ));

   m_gridP.resize( newSize, m_gridP.back() );
   for ( size_t i = oldSize; i < m_gridP.size(); ++i )
   {
      m_gridP[i] = m_gridP[i-1] + presStep;
      m_liqFrac.push_back( std::vector<double>( m_gridT.size(), -1.0 ) );
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Extend T grid to a new size. Also resize 2D liquid fraction array
// steps - number of points to be added
///////////////////////////////////////////////////////////////////////////////////////////////////
void PTDiagramCalculator::extendTGrid( int steps )
{
   size_t oldSize = m_gridT.size();

   m_gridT.resize( oldSize + steps, m_gridT.back() );
   for ( size_t i = oldSize; i < oldSize + steps; ++i )
   {
      m_gridT[i] = m_gridT[i-1] + m_gridT[i-1] - m_gridT[i-2];
   }

   for ( size_t pi = 0; pi < m_gridP.size(); ++pi )
   {
      m_liqFrac[pi].resize( m_gridT.size(), -1 );
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Call PVT library and calculate mole/volume or mass fraction for Liquid/Vapor phases for given composition for single set of P, T values
/// p pressure
/// t temperature
/// composition  array of mass values for each component
/// massFraction on return contains accumulated fractions for vapor/liquid phases (depends on m_diagType value)
/// return which phases exist in given composition for given P and T. "unknown" if call for flashing was failed
///////////////////////////////////////////////////////////////////////////////////////////////////
int PTDiagramCalculator::getMassFractions( double p, double t, const std::vector<double> & composition, double massFraction[2] )
{
   // Get acronims of some constants
   const int iNc     = ComponentId::NUMBER_OF_SPECIES;
   const int iNp     = PhaseId::NUMBER_OF_PHASES;
   const int iLiquid = PhaseId::LIQUID;
   const int iVapour = PhaseId::VAPOUR;
   
   // arrays for passing to flasher
   double masses[iNc];
   double phaseMasses[iNp][iNc];
   double phaseDensities[iNp] = {0.0, 0.0};

   assert( composition.size() == iNc );
   for ( int i = 0; i < iNc; ++i )
   {
      masses[i] = composition[i];
      for( int p = 0; p < iNp; ++p )
      {
         phaseMasses[p][i] = 0.0;
      }
   }

   // change the default behaviour for labeling phases for high temperature span
   if ( m_ChangeAoverB ) { m_flasher->setCritAoverBterm( m_AoverB ); }
   // increase precision for nonlinear solver
   m_flasher->setNonLinearSolverConvParameters( m_maxIters, m_stopTol, m_newtonRelCoeff ); 

   // Call flasher to get compositions for phases
   bool res = m_flasher->computeWithLumping( t, p, masses, phaseMasses, phaseDensities, NULL );  

   // revert back flasher settings
   if ( m_ChangeAoverB ) { m_flasher->resetToDefaultCritAoverBterm(); }
   m_flasher->setNonLinearSolverConvParameters();

   if ( !res ) return unknown;

   double totMass = std::accumulate( masses, masses + iNc, 0.0 ); // collect total mass for normalisation
   assert( totMass > 0.0 );

   double total = 0.0;

   switch( m_diagType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram: // convert to molar mass fraction
         {
            double gorm = m_flasher->gorm( masses );
            for ( int phase = 0; phase < iNp; ++phase )
            {
               for ( int comp = 0; comp < iNc; ++comp )
               {  
                  phaseMasses[phase][comp] /= m_flasher->getMolWeightLumped( comp, gorm );
                  total += phaseMasses[phase][comp];
               }
            }
         }
         break;

      case PTDiagramCalculator::VolumeFractionDiagram:
         for ( int phase = 0; phase < iNp; ++phase )
         {
            for ( int comp = 0; comp < iNc; ++comp )
            {  // convert from masses to volumes
               phaseMasses[phase][comp] = phaseDensities[phase] > 0.0 ? phaseMasses[phase][comp] / phaseDensities[phase] : 0.0;
               total += phaseMasses[phase][comp];
            }
         }
         break;

      default: // mass fraction
         total = totMass;
         break;
   }

   // normalize fractions
   for ( int phase = 0; phase < iNp; ++phase )
   {
      for ( int comp = 0; comp < iNc; ++comp )
      {
         phaseMasses[phase][comp] /= total;
      }
   }

   massFraction[iVapour] = std::accumulate( phaseMasses[iVapour], phaseMasses[iVapour] + iNc, 0.0 );
   massFraction[iLiquid] = std::accumulate( phaseMasses[iLiquid], phaseMasses[iLiquid] + iNc, 0.0 );

   // round values lower than tolerance to make some stabilisation of algorithm
   for ( int i = 0; i < 2; ++i )
   {
      if ( massFraction[i] < g_Tolerance * g_Tolerance )
      {
         massFraction[i] = 0.0;
      }
      if ( std::abs( 1.0 - massFraction[i] ) < g_Tolerance * g_Tolerance )
      {
         massFraction[i] = 1.0;
      }
   }
   
   return ((massFraction[iLiquid]) > 0.0 ? liquidPhase : unknown) + ((massFraction[iVapour] > 0.0) ? vaporPhase : unknown);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Do bisections for 2 given grid points to find bubble/dew curve
// p1 P grid the first point
// t1 T grid the first point
// p2 P grid the second point
// t2 T grid the second point
// foundP on success it contains bubble or dew point pressure value
// foundT on success it contains bubble or dew point temperature value
// return true if iterations were successful and bubble or dew point was found, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::doBisectionForBubbleDewSearch( size_t p1, size_t t1, size_t p2, size_t t2, double & foundP, double & foundT )
{
   int phase1 = getPhase( p1, t1 );
   int phase2 = getPhase( p2, t2 );

   bool bisectT = p1 == p2 && t1 != t2 ? true : false;
   if ( !bisectT && (p1 == p2 || t1 != t2) ) assert(0);
   
   // flasher failed for some reason, can't calculate bubble/dew point, or we do not have phase transition for (P1,T1)->(P2,T2)
   if ( unknown == phase1    || unknown == phase2    || phase1 == phase2 ) { return false; } 
   if ( phase1 != bothPhases && phase2 != bothPhases                     ) { return false; }

   double V1 = bisectT ? m_gridT[t1] : m_gridP[p1];
   double V2 = bisectT ? m_gridT[t2] : m_gridP[p2];

   foundP = m_gridP[p1];
   foundT = m_gridT[t1];
 
   for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
   {
      double phaseFrac[2];

      double V = 0.5 * ( V1 + V2 );

      int phase = getMassFractions( (bisectT ? foundP : V), (bisectT ? V : foundT), m_masses, phaseFrac ); // get phase composition for new guess
      ++m_bdBisecIters;

      if ( unknown == phase ) { return false; } // flasher failed

      if ( phase == phase1 )
      {
         V1 = V;
         phase1 = phase;
      }
      else if ( phase == phase2 )
      {
         V2 = V;
         phase2 = phase;
      }
      else if ( bothPhases != phase )
      {
         if ( bothPhases != phase1 )
         {
            V1 = V;
            phase1 = phase;
         }
         else 
         {
            V2 = V;
            phase2 = phase;
         }
      }

      if ( std::abs( V1 - V2 ) < m_eps * ( V1 + V2 ) ) // check convergence using relative tolerance
      {
         if ( bothPhases == phase1 ) // always return value outside of 2 phase region
         {
            if ( bisectT ) foundT = V2; 
            else           foundP = V2;
         } 
         else if ( bothPhases == phase2 )
         {
            if ( bisectT ) foundT = V1;
            else           foundP = V1;
         }
         else
         {
            if ( bisectT ) foundT = 0.5 * ( V1 + V2 );
            else           foundP = 0.5 * ( V1 + V2 );
         }
         return true;
      }
   }
   return false; // can't find bubble/dew point for maximum iterations
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Do bisections for 2 given grid points to find contour line
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::doBisectionForContourLineSearch( int p1, int t1, int p2, int t2, double frac, double & foundP, double & foundT )
{
   assert( p1 > -1 && p2 > -1 && t1 > -1 && t2 > -1 );
   assert( p1 < m_gridP.size() && p2 < m_gridP.size() && t1 < m_gridT.size() && t2 < m_gridT.size() );
   return doBisectionForContourLineSearch( m_gridP[p1], m_gridT[t1], m_gridP[p2], m_gridT[t2], getPhase( p1, t1 ), getPhase( p2, t2), 
          m_liqFrac[p1][t1], m_liqFrac[p2][t2], frac, foundP, foundT );
}

bool PTDiagramCalculator::doBisectionForContourLineSearch( double p1, double t1, double p2, double t2, double frac, double & foundP, double & foundT )
{
   const int iLiquid = PhaseId::LIQUID;
   const int iVapour = PhaseId::VAPOUR;

   double phaseFrac[2];

   int phase1 = getMassFractions( p1, t1, m_masses, phaseFrac );
   double frac1 = phaseFrac[iLiquid];

   int phase2 = getMassFractions( p2, t2, m_masses, phaseFrac );
   double frac2 = phaseFrac[iLiquid];
   m_isoBisecIters += 2;

   return doBisectionForContourLineSearch( p1, t1, p2, t2, phase1, phase2, frac1, frac2, frac, foundP, foundT );
}

bool PTDiagramCalculator::doBisectionForContourLineSearch( double p1, double t1, double p2, double t2, int phase1, int phase2, double frac1, double frac2, 
                                                           double frac, double & foundP, double & foundT )
{
   const int iLiquid = PhaseId::LIQUID;
   const int iVapour = PhaseId::VAPOUR;

   // flasher failed for some reason, can't calculate bubble/dew point, or we do not have phase transition for (P1,T1)->(P2,T2)
   if ( unknown == phase1 || unknown == phase2 ) { return false; } 

   // contour line comes outside of given segment
   if ( std::min( frac1, frac2 ) > frac || std::max( frac1, frac2 ) < frac || (phase1 == phase2 && bothPhases != phase1) )
   { 
      if ( std::abs( frac - frac1 ) < std::abs( frac - frac2 ) ) // chose nearest point
      {
         foundT = t1;
         foundP = p1;
      }
      else
      {
         foundT = t2;
         foundP = p2;
      }
      return false;
   }

   bool bisectT = std::abs( p1 - p2 ) < m_eps && std::abs( t1 - t2 ) > m_eps ? true : false;
   if ( !bisectT && ( std::abs( p1 - p2 ) < m_eps || std::abs( t1 - t2 ) > m_eps ) ) assert(0);

   double eps = bisectT ? m_epsT : m_epsP;

   double V1 = bisectT ? t1 : p1;
   double V2 = bisectT ? t2 : p2;

   foundT = t1;
   foundP = p1;

   double dirConv = 0;
   int    divergeSteps = -1;

   for ( int steps = 0; steps < g_MaxStepsNum && std::abs( V1 - V2 ) > eps; ++steps )
   {            
      double phaseFrac[2];

      // make the new guess
      double V = V1 + (frac - frac1)/(frac2 - frac1) * (V2 - V1);
      
      // can't find point
      if ( V < 0.0 ) { return false; }

      int phase = getMassFractions( (bisectT ? foundP : V), (bisectT ? V : foundT), m_masses, phaseFrac );
      ++m_isoBisecIters;
      
      // we get fraction value outside of [ min(frac1,frac2), max(frac1, frac2)] span
      // this means that function is not linear on given span
      // use linear aproximation in this case
      if ( std::min( frac1, frac2 ) > frac || std::max( frac1, frac2 ) < frac )
      {
         if ( bisectT ) foundT = V;
         else           foundP = V;
         return true;
      }

      if ( bothPhases == phase )
      {
         if( std::abs(phaseFrac[iLiquid] - frac) >= dirConv )
         {
            dirConv = std::abs(phaseFrac[iLiquid] - frac);
            divergeSteps++;
            if ( divergeSteps > 3 ) // algorithm diverged, just do linear interpolation
            {
              return false;
            }
         }
         else if ( divergeSteps > 0 )
         { 
            divergeSteps--;
         }
      }
      if ( bothPhases != phase ) // outside of the bubble/dew curve
      {  
         if ( phase == phase1 )
         {
            V1 = V;
            frac1 = phaseFrac[iLiquid]; 
         }
         else if ( phase == phase2 )
         {
            V2 = V;
            frac2 = phaseFrac[iLiquid];
         }
         else if ( bothPhases == phase1 && bothPhases == phase2 )
         {
            // use interpolation in this case
            if ( bisectT ) foundT = V;
            else           foundP = V;
            return true;
         }
         else // there is another phase between 1 phase region and 2 phase region. It is looks like a problem of EosPack
         {
           return false;
         }
      }
      else if ( (frac - phaseFrac[iLiquid]) * (frac1 - frac2) > 0 )
      {
         V2 = V;
         frac2 = phaseFrac[iLiquid]; 
         phase2 = phase;
      }
      else if ( (frac - phaseFrac[iLiquid]) * (frac1 - frac2) < 0 )
      {
         V1 = V;
         frac1 = phaseFrac[iLiquid];
         phase1 = phase;
      }
      else // by some accident we found the value!
      {
         if ( bisectT ) foundT = V;
         else           foundP = V;
         return true;
      }
    
      if ( std::abs( phaseFrac[iLiquid] - frac ) < m_eps * (phaseFrac[iLiquid] + frac) ) // checking using relative tolerance
      {
         if ( bisectT ) foundT = V;
         else           foundP = V;
         return true;
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Search by doing bisection iterations for single phases separation line values. Bisections could be done by the Pressure or by the Temperature
// p1 lower P border for bisections
// t1 lower T border for bisections
// p2 upper P border for bisections
// t2 upper T border for bisections
// foundP[out] on return, if iterations were successful, it contains bubble or dew point pressure value
// foundT[out] on return, if iterations were successful, it contains bubble or dew point temperature value
// return true if value was found, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::doBisectionForSinglePhaseSeparationLineSearch( size_t p1, size_t t1, size_t p2, size_t t2, double & foundP, double & foundT )
{
   int phase1 = getPhase( p1, t1 );
   int phase2 = getPhase( p2, t2 );

   bool bisectT = p1 == p2 && t1 != t2 ? true : false;
   if ( !bisectT && (p1 == p2 || t1 != t2) ) assert(0);

   double eps = bisectT ? m_epsT : m_epsP;

   // flasher failed for some reason, can't separation point, or we do not have phase transition for (P1,T1)->(P2,T2)
   if ( unknown == phase1 || unknown == phase2 || phase1 == phase2 ) { return false; } 

   double V1 = bisectT ? m_gridT[t1] : m_gridP[p1];
   double V2 = bisectT ? m_gridT[t2] : m_gridP[p2];

   foundP = m_gridP[p1];
   foundT = m_gridT[t1];

   for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
   {
      double phaseFrac[2];

      double V = 0.5 * ( V1 + V2 );

      int phase = getMassFractions( (bisectT ? foundP : V), (bisectT ? V : foundT), m_masses, phaseFrac ); // get phase composition for new guess
      ++m_bdBisecIters;

      if ( unknown == phase ) { return false; } // flasher failed

      if ( phase == phase1 )
      {
         V1 = V;
         phase1 = phase;
      }
      else if ( phase == phase2 )
      {
         V2 = V;
         phase2 = phase;
      }
      else //stop iterations 
      {
         V = (V1 + V2) * 0.5;
         V1 = V;
         V2 = V;
      }

      if ( std::abs( V1 - V2 ) < eps ) // check convergence using absolute tolerance
      {
         if ( bisectT ) foundT = 0.5 * ( V1 + V2 );
         else           foundP = 0.5 * ( V1 + V2 );
         return true;
      }
   }
   return false; // can't find bubble/dew point for maximum iterations
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate bubble/dew point lines, critical P and T and isoline for 0.5 fraction value
// compT trap composition temperature
// compP trap composition  pressure
// gridT 1D grid for temperature. If array is empty - the default grid is being used
///////////////////////////////////////////////////////////////////////////////////////////////////
void PTDiagramCalculator::findBubbleDewLines( double compT, double compP, const std::vector<double> & gridT )
{
   if ( !gridT.empty() )
   {
      m_gridT = gridT;
      for ( size_t i = 0; i < m_gridP.size(); ++i )
      {
         m_liqFrac[i].resize( m_gridT.size(), -1.0 );
      }
   }
   else 
   {  // adjust grid to keep given P and T for composition inside diagram
      if ( compT > m_gridT.back() ) { generatePTGrid( m_gridP.front(), m_gridP.back(), m_gridT.front(), compT ); }
      if ( compP > m_gridP.back() ) { generatePTGrid( m_gridP.front(), compP,          m_gridT.front(), m_gridT.back() ); }
   }

   m_bubbleDewLine.clear();
   m_c0p5Line.clear();
   m_spsLine.clear();

   const int iLiquid = PhaseId::LIQUID;
   const int iVapour = PhaseId::VAPOUR;

   // starting tracing Bubble/Dew point curve
   double foundT;
   double foundP;

   int    t1 = -1;
   int    p1 = -1;
   int    lgrPos = -1;
   int    inEdge = -1;

   // as first step do fraction calculation on the grid T and P till we'll find bubble/dew line starting point
   for ( int  pi = 0; pi < m_gridP.size()-1 && p1 < 0; ++pi )
   {
      for ( int ti = 0; ti < m_gridT.size()-1 && t1 < 0; ++ti )
      {
         if ( getPhase( pi, ti ) != getPhase( pi+1, ti ) )
         {
            if ( doBisectionForBubbleDewSearch( pi, ti, pi+1, ti, foundP, foundT ) )
            {
               t1 = ti;
               p1 = pi;
               inEdge = 0;
               m_bubbleDewLine.push_back( TPPoint( foundT, foundP ) );
            }
         }
         else if ( getPhase( pi, ti ) != getPhase( pi, ti+1 ) )
         {
            if ( doBisectionForBubbleDewSearch( pi, ti, pi, ti+1, foundP, foundT ) )
            {
               t1 = ti;
               p1 = pi;
               inEdge = 3;
               m_bubbleDewLine.push_back( TPPoint( foundT, foundP ) );
            }
         }
      }
   }

   // scanned all grid by can't find bubble/dew line - can't build diagram
   if ( t1 >= 0 || p1 >= 0 )
   {
      std::set<size_t>  trace; // for keeping cells through which isoline came across

      // now we can trace bubble-dew line
      bool foundBDPt = true;
      while( foundBDPt )
      {
         if ( t1 + 1 == m_gridT.size() ) { extendTGrid( g_GridExtStep ); }
         if ( p1 + 1 == m_gridP.size() ) { extendPGrid( m_gridP[p1] + ( m_gridP[p1-1] - m_gridP[p1-2] ) * g_GridExtStep ); }

         if ( m_gridP.back() > g_MaximalDiagramPressure ) foundBDPt = false; // if we came too much in pressure stop tracing

         int phase[4];

         phase[0] = getPhase( p1,   t1   );
         phase[1] = getPhase( p1+1, t1   );
         phase[2] = getPhase( p1+1, t1+1 );
         phase[3] = getPhase( p1,   t1+1 );

         bool inters[4] = { false, false, false, false };
         
         for ( int i = 0; i < 4; ++i )
         {
            if ( phase[i] != phase[(i+1)%4] )
            {
               // skip vapour/liquid interface near critical point
               inters[i] = phase[i] != bothPhases && phase[(i+1)%4] != bothPhases ? false : true;
            }
         }
         assert( inters[inEdge] ); // just checking if we have right input point

         if ( foundBDPt )
         {
            for ( int i = 0; i < 4; ++i )
            {
               if ( inters[i] && i != inEdge )
               {
                  switch( i )
                  {
                     case 0: foundBDPt = doBisectionForBubbleDewSearch( p1,   t1,   p1+1, t1,   foundP, foundT ); break;
                     case 1: foundBDPt = doBisectionForBubbleDewSearch( p1+1, t1,   p1+1, t1+1, foundP, foundT ); break;
                     case 2: foundBDPt = doBisectionForBubbleDewSearch( p1,   t1+1, p1+1, t1+1, foundP, foundT ); break;
                     case 3: foundBDPt = doBisectionForBubbleDewSearch( p1,   t1,   p1,   t1+1, foundP, foundT ); break;
                  }

                  if ( foundBDPt )
                  {
                     m_bubbleDewLine.push_back( TPPoint( foundT, foundP ) );

                     switch( i ) // choose next cell
                     {
                        case 0: --t1; break;
                        case 1: ++p1; break;
                        case 2: ++t1; break;
                        case 3: --p1; break;
                     }

                     if ( t1 < 0 || p1 < 0 ) foundBDPt = false;

                     // do checking is this cell was already in trace ? if yes - stop tracing
                     foundBDPt = foundBDPt && trace.insert( p1 * 10000 + t1 ).second ;

                     inEdge = (i + 2) %4;
                     break;
                  }
               }
            }
         }
      }
   }   
   // trace 0.5 contour line at first
   m_c0p5Line = calcContourLine( 0.5 );
   findCriticalPoint();

   // check the direction of Bubble/Dew line
   if ( m_bubbleDewLine.size() > 1 )
   {
      double phaseFrac[2];
      int phaseF = getMassFractions( m_bubbleDewLine.front().second, m_bubbleDewLine.front().first, m_masses, phaseFrac ); 
      int phaseL = getMassFractions( m_bubbleDewLine.back().second, m_bubbleDewLine.back().first, m_masses, phaseFrac ); 

      if ( phaseF != bothPhases && phaseL != bothPhases && phaseL == vaporPhase && phaseF == liquidPhase ) // should revert points ordering
      {
         size_t critPPos = m_critPointPos - m_bubbleDewLine.begin();
         std::reverse( m_bubbleDewLine.begin(), m_bubbleDewLine.end() );
         m_critPointPos = m_bubbleDewLine.end() - 1 - critPPos;
      }
   }

   // it is possible that there is no any 2 phase region if only 1 component compostion
   // in this case we will try to find buble/dew line as part of phases separation line
   if ( !m_bubbleDewLine.size() && !m_c0p5Line.size() )
   {
      const TPLine & psl = getSinglePhaseSeparationLine();
      if ( psl.size() )
      {
         size_t critPtPos = 1;
         // drop vertical part if exist
         while( critPtPos < psl.size() && std::abs(psl[critPtPos-1].first - psl[critPtPos].first) < m_eps ) ++critPtPos;
         size_t blp = critPtPos;
         for ( critPtPos += 1; critPtPos < psl.size()-1; ++critPtPos )
         {
            // drop linear part
            if ( std::abs( (psl[critPtPos  ].second - psl[blp].second)*1e-6 / (psl[critPtPos  ].first - psl[blp].first) - 
                           (psl[critPtPos+1].second - psl[blp].second)*1e-6 / (psl[critPtPos+1].first - psl[blp].first)) > m_eps*10.0 )
               break;
         }
         // the first point must be critical point
         if ( critPtPos < psl.size() - 1 )
         {
            m_critT = psl[critPtPos].first;
            m_critP = psl[critPtPos].second;
            m_bubbleDewLine.assign( psl.rbegin(), psl.rend()-critPtPos );
            m_spsLine.resize( critPtPos );
            m_critPointPos = m_bubbleDewLine.end() - 1;
         }
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Find critical point as intersection of 0.5 isoline with bubble/dew curve. If not found - go along bubble/dew curve
// and look for phase changed from gas to liquid
//
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::findCriticalPointInContourBubbleDewLinesIntersection()
{
   bool foundCriticalPoint = false;

   double scaleT = 1.0 / (m_gridT.back() - m_gridT.front());
   double scaleP = 1.0 / (m_gridP.back() - m_gridP.front());

   if ( m_c0p5Line.size() > 3 ) // find critical point on to Bubble/Dew curve
   {  double minDist = 2;

      // convert to 0-1 segment
      double critT = ( m_c0p5Line.back().first  - m_gridT.front() ) * scaleT;
      double critP = ( m_c0p5Line.back().second - m_gridP.front() ) * scaleP;

      for ( TPLine::iterator it = m_bubbleDewLine.begin(); it != m_bubbleDewLine.end(); ++it )
      {
         // convert to 0-1 segment
         double T = ( it->first  - m_gridT.front()) * scaleT;
         double P = ( it->second - m_gridP.front()) * scaleP;
         double dist = std::sqrt( ( T - critT ) * ( T - critT ) + ( P - critP ) * ( P - critP ) );

         if ( dist < minDist )
         {
            minDist = dist;
            m_critPointPos = it;
         }
      }

      if ( m_critPointPos != m_bubbleDewLine.end() )
      {
         // check is found nearest bubble/dew point not too far from critical point
         if ( std::abs( m_critPointPos->first  - m_c0p5Line.back().first  ) < ( m_gridT[1] - m_gridT[0] ) &&
              std::abs( m_critPointPos->second - m_c0p5Line.back().second ) < ( m_gridP[1] - m_gridP[0] ) 
            )
         {
            // and also it is not already on bubble dew curve
            if ( (( m_critPointPos->first  - m_c0p5Line.back().first ) > m_epsT) || (( m_critPointPos->second - m_c0p5Line.back().second ) > m_epsP) )
            {
               m_critT = m_c0p5Line.back().first;
               m_critP = m_c0p5Line.back().second;
               m_critPointPos = m_bubbleDewLine.insert( (m_critPointPos->first > m_critT ? m_critPointPos : (m_critPointPos + 1)), TPPoint( m_critT, m_critP ) );
            }
            else
            {
               m_critT = m_critPointPos->first;
               m_critP = m_critPointPos->second;
            }
            foundCriticalPoint = true;
         }
      }
   }
   return foundCriticalPoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Find critical point as single phase change point along bubble/dew line
// addToBubblDewLine add found value to bubble/dew line
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::findCriticalPointInChangePhaseAlongBubbleDewLine( bool addToBubblDewLine )
{
   bool foundCriticalPoint = false;

   if ( !m_bubbleDewLine.size() ) return foundCriticalPoint;

   double scaleT = 1.0 / (m_gridT.back() - m_gridT.front());
   double scaleP = 1.0 / (m_gridP.back() - m_gridP.front());


   double massFraction[2];
   int phase1 = getMassFractions( m_bubbleDewLine.front().second, m_bubbleDewLine.front().first, m_masses, massFraction );
   ++m_bdBisecIters;

   // save current position   
   TPLine::iterator  old_critPointPos = m_critPointPos;

   m_critPointPos = m_bubbleDewLine.begin();
   for ( TPLine::iterator it = m_critPointPos + 1; it != m_bubbleDewLine.end() && !foundCriticalPoint; ++it )
   {
      int phase2 = getMassFractions( it->second, it->first, m_masses, massFraction );
      ++m_bdBisecIters;

      if ( phase1 != phase2 && phase1 != bothPhases && phase2 != bothPhases )
      {
         double P1 = m_critPointPos->second;
         double T1 = m_critPointPos->first;
         double P2 = it->second;
         double T2 = it->first;

         for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
         {
            double newP = 0.5 * (P1 + P2);
            double newT = 0.5 * (T1 + T2);

            int newPhase = getMassFractions( newP, newT, m_masses, massFraction );
            ++m_bdBisecIters;

            if ( phase1 == newPhase )
            {
               P1 = newP;
               T1 = newT;
            }
            else if ( phase2 = newPhase )
            {
               P2 = newP;
               T2 = newT;
            }
            else
            {
               break;
            }

            // check convergence
            if ( std::max( std::abs( P1 - P2 ) * scaleP, std::abs( T1 - T2 ) * scaleT ) < m_eps )
            {
               m_critT = 0.5 * ( T1 + T2 );
               m_critP = 0.5 * ( P1 + P2 );

               if ( addToBubblDewLine )
               {
                  m_critPointPos = m_bubbleDewLine.insert( it, TPPoint( m_critT, m_critP ) );
               }
               foundCriticalPoint = true;
               break;
            }
         }
      }
      else
      {
         phase1 = phase2;
         m_critPointPos = it;
      }
   }
   
   if ( !addToBubblDewLine ) m_critPointPos = old_critPointPos;

   return foundCriticalPoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Find critical pont by calling findCriticalPointInContourBubbleDewLinesIntersection() first, if it wasn't found
// then call findCriticalPointInChangePhaseAlongBubbleDewLine()
//
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::findCriticalPoint()
{
   m_critPointPos = m_bubbleDewLine.end();
   
   bool foundCriticalPoint = findCriticalPointInContourBubbleDewLinesIntersection();

   // if critical point wasn't found, try to find critical point by tracking changing phases from vapor to liquid along buble/dew line
   if ( !foundCriticalPoint && m_bubbleDewLine.size() > 2 )
   {
      foundCriticalPoint = findCriticalPointInChangePhaseAlongBubbleDewLine();
   }
   return foundCriticalPoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate contour line for given value. Should be called after findBubleDewLines().
// val Liquid fraction value for contour line. Must be between 0 and 1
// returns array of (T,P) pair for each contour line point
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::TPLine PTDiagramCalculator::calcContourLine( double val )
{
   TPLine isoline;

   if ( std::abs( val - 1.0 ) < m_eps ) // request for bubble curve
   {
      for ( int ti = static_cast<int>(m_bubbleDewLine.size()) - 1; ti >= (m_critPointPos - m_bubbleDewLine.begin()); --ti )
      {
         isoline.push_back( m_bubbleDewLine[ti] );
      }
      return isoline;
   }
   
   if ( val < m_eps && m_critPointPos != m_bubbleDewLine.end() ) // request for dew curve
   {
      for ( int ti = static_cast<int>(m_critPointPos - m_bubbleDewLine.begin()); ti >= 0; --ti )
      {
         isoline.push_back( m_bubbleDewLine[ti] );
      }
      return isoline;
   }

   if ( std::abs( val - 0.5 ) < m_eps && m_c0p5Line.size() ) // request for 0.5 contour line
   {
      return m_c0p5Line;
   }

   // trace contour line using bisections
   traceContourLine( val, isoline, false );
   return isoline;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Calculate set of contour lines for the given values set. Should be called after findBubleDewLines().
/// vals the set of liquid fraction values for contour line. Must be between 0 and 1
/// return array of (T,P) pair for each contour line point. Countour lines are divided by (-1,-1) pair values
///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<double> PTDiagramCalculator::calcContourLines( std::vector<double> & vals )
{
   std::vector<double> valsSet;
   for ( std::vector<double>::iterator v = vals.begin(); v != vals.end(); ++v )
   {
      assert( (*v) >= 0.0 && (*v) <= 1.0 );
      const TPLine & isoline = calcContourLine( *v );
      for ( TPLine::const_iterator it = isoline.begin(); it != isoline.end(); ++it )
      {
         valsSet.push_back( it->first );
         valsSet.push_back( it->second );
      }

      valsSet.push_back( -1.0 );
      valsSet.push_back( -1.0 );
   }
   return valsSet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// calculate (if it wasn't done before) liquid fraction value for given P & T points on the grid
/// p pressure grid position 
/// t temperature grid position
/// return liquid fraction value
///////////////////////////////////////////////////////////////////////////////////////////////////
double PTDiagramCalculator::getLiquidFraction( int p, int t )
{
   double val = -1.0;
   
   if ( p > -1 && p < m_gridP.size() && t > -1 && t < m_gridT.size() )
   {
      getPhase( p, t );
      val = m_liqFrac[p][t];
   }
   return val;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// This enum describes how given cell on P/T grid is located on phase diagram
///////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
   DontKnow                 = -1, // can't define position
   AllInOnePhaseRegion      =  0, // all corners of cell in 1 phase region
   AllInTwoPhaseRegionLess  =  1, // all corners of cell in 2 phase region and all values in corners smoller then given value
   AllInTwoPhaseRegionMore  =  2, // all corners of cell in 2 phase region and all values in corners greater then given value
   AllInTwoPhaseRegionCross =  3, // all corners of cell in 2 phase region and som values in corners less then given value and some greater
   OneTwoPhaseRegionCross   =  4  // some cells corner in 1 phase region, some in 2 two phase region
} CellPositionOnDiagram;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Find cell position on P/T phase diagram
//
// p1  low left cell corner P value
// p2  upper right cell corner P value
// t1  low left cell corner T value
// t2  upper right cell corner T value
// val countour line value
//
// return cell position on phase diagram (1/2 phase region) or DontKnow if it is not simple to find the position
///////////////////////////////////////////////////////////////////////////////////////////////////
int PTDiagramCalculator::checkCell( size_t p1, size_t p2, size_t t1, size_t t2, double val )
{
   int phase[4];
   phase[0] = getPhase( p1, t1 );
   phase[1] = getPhase( p2, t1 );
   phase[2] = getPhase( p2, t2 );
   phase[3] = getPhase( p1, t2 );
   
   double liqFrac[4];
   liqFrac[0] = m_liqFrac[p1][t1];
   liqFrac[1] = m_liqFrac[p2][t1];
   liqFrac[2] = m_liqFrac[p2][t2];
   liqFrac[3] = m_liqFrac[p1][t2];

   int num1Phase = 0;
   int numLiqPhase = 0;
   int numVapPhase = 0;
   int num2Phase = 0;
   int num2PhaseLessVal = 0;
   int num2PhaseMoreVal = 0;

   for ( int i = 0; i < 4; ++i )
   {
      if ( bothPhases == phase[i] )
      {
         ++num2Phase;
         if (      liqFrac[i] <= val ) ++num2PhaseLessVal;
         else if ( liqFrac[i] >  val ) ++num2PhaseMoreVal;
      }
      else if ( liquidPhase == phase[i] ) { ++num1Phase; ++numLiqPhase; }
      else if ( vaporPhase  == phase[i] ) { ++num1Phase; ++numVapPhase; }
   }
   
   if ( 4 == num1Phase ) 
   {
      return std::max( p2 - p1, t2 - t1 ) > 20 ? DontKnow : AllInOnePhaseRegion; // 4 points in 1 phase region (for very big cell could be not right)
   }
   if ( 4 == num2Phase )
   {
      if ( 4 == num2PhaseLessVal ) return AllInTwoPhaseRegionLess; // 4 points in 2 phase region but all nodes have smaller value than given
      if ( 4 == num2PhaseMoreVal ) return AllInTwoPhaseRegionMore; // 4 points in 2 phase return but all nodes have bigger value than given
      return AllInTwoPhaseRegionCross;                             // some points smaller than given value some greater
   }

   if ( num2Phase > 0 && (numLiqPhase + numVapPhase) > 0 && 
        num2PhaseLessVal > 0 && num2PhaseMoreVal > 0 ) return OneTwoPhaseRegionCross; // some points in  1 phase and some in 2 phases
   
   return DontKnow; // can't say something 
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Trace countour line for given liquid fraction value. If fast is true, trace only till intersection with bubble/dew line
//
/// val           liquid fraction value for contour line
/// isoloine[out] on output contour line as array of (T,P) points
/// fast          if true, trace isoline till intersection with bubble/dew line only (used for search of critical point)
//
/// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
struct CellInds
{
   int t; // temperature grid pos
   int p; // pressure grid pos
   int l; // level of refinement
   int c; // corner of refined cell 1-4

   CellInds( int tt, int pp, int ll, int cc ) : t(tt), p(pp), l(ll), c(cc) {;}
};

bool operator < ( const CellInds & cp1, const CellInds & cp2 ) 
{  
   if (      cp1.t < cp2.t ) return true;
   else if ( cp1.t > cp2.t ) return false;
   
   if (      cp1.p < cp2.p ) return true;
   else if ( cp1.p > cp2.p ) return false;

   if (      cp1.l < cp2.l ) return true;
   else if ( cp1.l > cp2.l ) return false;

   if (      cp1.c < cp2.c ) return true;
   else if ( cp1.c > cp2.c ) return false;

   return false;
}


bool PTDiagramCalculator::traceContourLine( double val, TPLine & isoline, bool fast )
{
   // corners ordering:    edges ordering:
   //                           1
   //  1 *------* 2         *-------*
   //    |      |         0 |       | 2 
   //  0 *------* 3         *-------*
   //                          3

   // find starting point
   int t1     = -1;
   int p1     = -1;
   int lgrPos = -1;
   int inEdge = -1;

   double foundT = 0.0;
   double foundP = 0.0;

   // do search on coarse grid
   std::stack<SuperCell> stack;

   stack.push( SuperCell( 1, m_gridP.size() - 1, 1, m_gridT.size() - 1 ) );

   SuperCell foundCl( 99999, 99999, 99999, 99999 );
   SuperCell secondChanceCl( 99999, 99999, 99999, 99999 );

   for ( bool found = false; !found && stack.size() > 0;  )
   {
      size_t ss = stack.size();
      SuperCell cc = stack.top();
      stack.pop();

      CellPositionOnDiagram ret = static_cast<CellPositionOnDiagram>( checkCell( cc.p1, cc.p2, cc.t1, cc.t2, val ) );
      switch( ret )
      {
         case OneTwoPhaseRegionCross:
            if ( 2 == (cc.p2 - cc.p1 + cc.t2 - cc.t1 ) ) { secondChanceCl = cc; break; }
         case DontKnow: // split cell and push to stack
            {
               size_t p = (cc.p2 - cc.p1) > 1 ? cc.p1 + (cc.p2 - cc.p1) / 2 : cc.p2;
               size_t t = (cc.t2 - cc.t1) > 1 ? cc.t1 + (cc.t2 - cc.t1) / 2 : cc.t2;
               if ( p != cc.p2 || t != cc.t2 )
               {
                  if ( (cc.p2 - p)     > 0 && (t     - cc.t1) > 0 ) stack.push( SuperCell( p,     cc.p2, cc.t1, t  ) );
                  if ( (p     - cc.p1) > 0 && (cc.t2 - t)     > 0 ) stack.push( SuperCell( cc.p1, p,     t,     cc.t2 ) );
                  if ( (cc.p2 - p)     > 0 && (cc.t2 - t)     > 0 ) stack.push( SuperCell( p,     cc.p2, t,     cc.t2 ) );
                  if ( (p     - cc.p1) > 0 && (t     - cc.t1) > 0 ) stack.push( SuperCell( cc.p1, p,     cc.t1, t  ) );
               }
            }
            break;

         case AllInTwoPhaseRegionCross:
            if ( 2 == (cc.p2 - cc.p1 + cc.t2 - cc.t1 ) )
            {
               foundCl = cc;
               found = true;
            }
            else if ( (cc.p2 - cc.p1 + cc.t2 - cc.t1 ) > 2 ) // still many cells inside split by 2x2
            {
               size_t p = (cc.p2 - cc.p1) > 1 ? cc.p1 + (cc.p2 - cc.p1) / 2 : cc.p2;
               size_t t = (cc.t2 - cc.t1) > 1 ? cc.t1 + (cc.t2 - cc.t1) / 2 : cc.t2;
               if ( p != cc.p2 || t != cc.t2 )
               {
                  if ( (cc.p2 - p)     > 0 && (t     - cc.t1) > 0 ) stack.push( SuperCell( p,     cc.p2, cc.t1, t  ) );
                  if ( (p     - cc.p1) > 0 && (cc.t2 - t)     > 0 ) stack.push( SuperCell( cc.p1, p,     t,     cc.t2 ) );
                  if ( (cc.p2 - p)     > 0 && (cc.t2 - t)     > 0 ) stack.push( SuperCell( p,     cc.p2, t,     cc.t2 ) );
                  if ( (p     - cc.p1) > 0 && (t     - cc.t1) > 0 ) stack.push( SuperCell( cc.p1, p,     cc.t1, t  ) );
               }
            }
            break;

         default:
           break;
      }
   }
   // if we found cell intersected by contour line, set it as start cell
   if ( foundCl.p1 < m_gridP.size() && foundCl.t1 < m_gridT.size() )
   {
      t1 = static_cast<int>(foundCl.t1);
      p1 = static_cast<int>(foundCl.p1);
   }
   else if ( secondChanceCl.p1 < m_gridP.size() && secondChanceCl.t1 < m_gridT.size() )
   {  // if we can't find cell in two phase region which is crossed by isoline, try to use 1/2 phase cell crossed by isoline
      t1 = static_cast<int>(secondChanceCl.t1);
      p1 = static_cast<int>(secondChanceCl.p1);
      foundCl = secondChanceCl;
   }
   
   bool forward = true; // starting trace in one direction from found cell, then will try another side also
   int  startEdge = -1; // remember from which edge tracing was started, to continue in another direction

   // scanned all grid by can't find cell which was crossed by given countour line
   if ( t1 < 0 && p1 < 0 ) { return false; }

   isoline.clear();      // clear isoline, just in case

   std::set<CellInds>  trace; // for keeping cells through which isoline came across

   int    level = 0;      // level of grid refining

   // will be used in cell refining near bubble/dew line
   double minP;
   double minT;
   double maxP;
   double maxT;

   // main tracing cycle
   bool   foundPt = true;
   while( foundPt )
   {
      if ( p1 + 1 == m_gridP.size() && m_gridP.back() < g_MaximalDiagramPressure ) { extendPGrid( m_gridP[p1] + ( m_gridP[p1-1] - m_gridP[p1-2] ) * g_GridExtStep ); } 

      if ( t1 + 1 == m_gridT.size() || p1 + 1 == m_gridP.size() ) // if we went to the grid limits, stop and try another side 
      {
         if ( forward && !fast ) // should we trace other side of the contour line?
         {
            p1 = static_cast<int>(foundCl.p1);
            t1 = static_cast<int>(foundCl.t1);
            forward = false;
            inEdge = startEdge;
            std::reverse( isoline.begin(), isoline.end() );
            level = 0;
            continue;
         }
         else { break; } // can't trace outside grid
      } 
      
      int    phase[4];    // which phase for each cell corner
      double fracVals[4]; // what is liquid fraction value for each cell corner

      // check that all values for cell corners are exist
      if ( !level ) // top level - get values from the grid
      {
         phase[0] = getPhase( p1,   t1   );
         phase[1] = getPhase( p1+1, t1   );
         phase[2] = getPhase( p1+1, t1+1 );
         phase[3] = getPhase( p1,   t1+1 );          
      
         fracVals[0] = m_liqFrac[p1  ][t1  ]; 
         fracVals[1] = m_liqFrac[p1+1][t1  ];
         fracVals[2] = m_liqFrac[p1+1][t1+1];
         fracVals[3] = m_liqFrac[p1  ][t1+1];

         // initialise top level cell if we will need to refine it
         minP = m_gridP[p1];
         maxP = m_gridP[p1+1];
         minT = m_gridT[t1];
         maxT = m_gridT[t1+1];
      }
      else
      {  // refined cell, calculate liquid fraction values for each corner
         for ( int i = 0; i < 4; ++i )
         {
            double phaseFrac[2];
            switch( i )
            {
               case 0: phase[i] = getMassFractions( minP, minT, m_masses, phaseFrac ); break;
               case 1: phase[i] = getMassFractions( maxP, minT, m_masses, phaseFrac ); break;
               case 2: phase[i] = getMassFractions( maxP, maxT, m_masses, phaseFrac ); break;
               case 3: phase[i] = getMassFractions( minP, maxT, m_masses, phaseFrac ); break;
            }
            fracVals[i] = phaseFrac[PhaseId::LIQUID];
            ++m_isoBisecIters;            
         }
      }

      bool inters[4] = { false, false, false, false };
      int intersNum = 0; // number of intersection by contour line

      for ( int i = 0; i < 4; ++i )
      {
         if ( bothPhases == phase[i] && bothPhases == phase[(i+1)%4] )
         {
            if ( !inters[i] && Between( val, fracVals[i], fracVals[(i+1)%4] ) )
            {
               inters[i] = true;
               intersNum++;
            }
         }
      }

      if ( startEdge < 0 ) // if first cell in trace set inEdge low and startEdge up to trace in direction of bubble/dew line
      {
         if ( intersNum > 1 )
         {
            if (      inters[1] ) startEdge = 1;
            else if ( inters[2] ) startEdge = 2;
            else if ( inters[0] ) startEdge = 0;
            else assert(0);

            for ( int i = 0; i < 4 && inEdge < 0; ++i ) if ( i != startEdge && inters[i] ) inEdge = i;
            assert( inEdge != -1 );
         }
      }

      if ( intersNum < 2 ) // only one intersection point with cell edges - stop or refine cell
      {
         if ( level >= g_maxLGRLevel && forward && !fast ) // no output edge, trace the second part of curve
         {
            p1 = static_cast<int>(foundCl.p1);
            t1 = static_cast<int>(foundCl.t1);
            forward = false;
            inEdge = startEdge;
            std::reverse( isoline.begin(), isoline.end() );
            level = 0;
            continue;
         }

         if ( level < g_maxLGRLevel ) // if maximum level of refinement wasn't reached, refine cell 2x2
         {
            level++;
            // split cell on 4 parts. Use central point for splitting
            double midP = 0.5 * ( minP + maxP );
            double midT = 0.5 * ( minT + maxT );
           
            // check which subcell we shall chose
            if ( foundT + m_epsT > minT && foundT - m_epsT < midT && foundP + m_epsP > minP && foundP - m_epsP < midP ) // low left cell
            {
               lgrPos = 1;
               maxP = midP;
               maxT = midT;
            }
            else if ( foundT + m_epsT > minT && foundT - m_epsT < midT && foundP + m_epsP > midP && foundP - m_epsP < maxP ) // upper left cell
            {
               lgrPos = 2;
               minP = midP;
               maxT = midT;
            }
            else if ( foundT + m_epsT > midT && foundT - m_epsT < maxT && foundP + m_epsP > midP && foundP - m_epsP < maxP ) // upper right cell
            {
               lgrPos = 3;
               minP = midP;
               minT = midT;
            }
            else // low right cell
            {
               lgrPos = 4;
               maxP = midP;
               minT = midT;
            }
            continue; // restart loop with new subcell
         }
      }

      foundPt = false;
      for ( int i = 0; i < 4; ++i ) // loop over cell edges to find out output edge and intersection point
      {
         // try all intersected edges other than inEdge to find out point
         if ( inters[i] && i != inEdge )
         {
            if ( level ) // refined cell - using min/max P/T values for corners
            {
               switch( i )
               {
                  case 0: foundPt = doBisectionForContourLineSearch( minP, minT, maxP, minT, val, foundP, foundT ); break;
                  case 1: foundPt = doBisectionForContourLineSearch( maxP, minT, maxP, maxT, val, foundP, foundT ); break;
                  case 2: foundPt = doBisectionForContourLineSearch( minP, maxT, maxP, maxT, val, foundP, foundT ); break;
                  case 3: foundPt = doBisectionForContourLineSearch( minP, minT, minP, maxT, val, foundP, foundT ); break;
               }
            }
            else // top level cell - using cell corners on P/T grid
            {
               switch( i )
               {
                  case 0: foundPt = doBisectionForContourLineSearch( p1,   t1,   p1+1, t1,   val, foundP, foundT ); break;
                  case 1: foundPt = doBisectionForContourLineSearch( p1+1, t1,   p1+1, t1+1, val, foundP, foundT ); break;
                  case 2: foundPt = doBisectionForContourLineSearch( p1,   t1+1, p1+1, t1+1, val, foundP, foundT ); break;
                  case 3: foundPt = doBisectionForContourLineSearch( p1,   t1,   p1,   t1+1, val, foundP, foundT ); break;
               }
            }
            if ( foundPt ) isoline.push_back(  TPPoint( foundT, foundP ) ); // add output point to list

            if ( !level ) // if top level cell - go to next cell 
            {
               switch( i ) // choose next cell
               {
                  case 0: --t1; break;
                  case 1: ++p1; break;
                  case 2: ++t1; break;
                  case 3: --p1; break;
               }
            }
            else // if refined cell shift to another subcell staying on the same level
            {
               double dT = maxT - minT;
               double dP = maxP - minP;
               switch( i ) //
               {
                  case 0: minT -= dT; maxT -= dT; break;
                  case 1: minP += dP; maxP += dP; break;
                  case 2: minT += dT; maxT += dT; break;
                  case 3: minP -= dP; maxP -= dP; break;
               }
               // check if we went outside top level cell, in this case stop refining and go to next top level cell
               if (      minT + m_epsT < m_gridT[t1  ] ) { level = 0; t1--; }
               else if ( maxT - m_epsT > m_gridT[t1+1] ) { level = 0; t1++; }
               else if ( minP + m_epsP < m_gridP[p1  ] ) { level = 0; p1--; }
               else if ( maxP - m_epsP > m_gridP[p1+1] ) { level = 0; p1++; }
            }
            if ( !level && (t1 < 0 || p1 < 0) ) foundPt = false;
            
            // do checking is this cell was already in trace ? if yes - stop tracing
            foundPt = foundPt && trace.insert( CellInds(t1, p1, level, lgrPos) ).second ;

            inEdge = (i + 2) %4;
            break;
         }
      }
      if ( !foundPt && forward && !fast ) // if didn't find next cell to continue - try to trace from other side of the countour line
      {
         forward = false;
         t1 = static_cast<int>( foundCl.t1 );
         p1 = static_cast<int>( foundCl.p1 );
         inEdge = startEdge;
         std::reverse( isoline.begin(), isoline.end() );
         level = 0;
         foundPt = true;
      }
   }
   
   if ( isoline.size() && isoline.front().second > isoline.back().second ) reverse( isoline.begin(), isoline.end() ); // put isoline in order with high P last

   // finding critical point by finding bubble/dew point on extrapolation of contour line
   if ( isoline.size() > 1 ) 
   {
      size_t last = isoline.size()-1;
      foundPt = extrapolateContourLineToOnePhaseRegion( isoline, isoline[last-1].second, isoline[last-1].first, isoline[last].second, isoline[last].first, foundP, foundT ); 
      if ( foundPt )
      {
         isoline.push_back( TPPoint( foundT, foundP ) ); // if found 2/1 phase change point - add it to the curve
      }
   }
   return foundPt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Find A/B term value in such way that liquid/vapour division line  will come through critical point
// 
// return found value of A/B term if it was found or the default value if not found (0.5 countour 
//        line doesn't cross bubble/dew line)
///////////////////////////////////////////////////////////////////////////////////////////////////
double PTDiagramCalculator::searchAoverBTerm()
{
   double critT = m_critT;
   bool foundPt = true;

   if ( !m_c0p5Line.size() )
   {
      TPLine c0p5Line;
      foundPt = traceContourLine( 0.5, c0p5Line, true );
      if ( foundPt )
      {
         critT = c0p5Line.back().first;
         m_critT = critT;
         m_critP = c0p5Line.back().second;
      }
      else // can't find 0.5 isoline, try another one
      {
         for ( double v = 0.9; !foundPt && v > 0.0; v -= 0.1 )
         {  
            if ( std::abs( v - 0.5 ) < m_eps ) continue; // ignore 0.5 isoline
            TPLine isol;
            foundPt = traceContourLine( v, isol, true );
            if ( foundPt )
            {
               critT = isol.back().first;
               m_critT = critT;
               m_critP = isol.back().second;
            }
         }
      }
   }

   if ( foundPt )
   {
      double oldAoverB = m_AoverB;

     // get cell in which critical point exist
      int critI = static_cast<int>( std::floor( (critT - m_gridT.front())/(m_gridT[1] - m_gridT[0] ) ) );

      if ( 0 >= critI || critI >= m_gridT.size()-2 ) return m_AoverB;

      double ltp = m_gridT[critI];
      double rtp = m_gridT[critI+1];
      double pp  = m_gridP.back();

      double minAoverB = 1.0;
      double maxAoverB = 10.0;

      for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
      {
         double massFraction[2];

         int phase1 = getMassFractions( pp, ltp, m_masses, massFraction );
         int phase2 = getMassFractions( pp, rtp, m_masses, massFraction );
         m_abTuneBisecIters += 2;

         if ( bothPhases == phase1 || bothPhases == phase2 ) // too low P needed go up
         {
            pp *= 1.1;
            steps--;
            if ( pp > 200e6 ) // already come too far. stop iterations
            {
               m_AoverB = oldAoverB;
               return m_AoverB;
            }
            continue;
         }
         if ( phase1 < liquidPhase || phase1 > vaporPhase || phase2 < liquidPhase || phase2 > vaporPhase ) // fail to search
         {
            m_AoverB = oldAoverB;
            return m_AoverB;
         }

         if ( phase1 == phase2 )
         {
            if ( phase1 == liquidPhase ) { minAoverB = m_AoverB; }
            else                         { maxAoverB = m_AoverB; }            

            m_AoverB = (maxAoverB + minAoverB) * 0.5;
         }
         else
         {
            ltp += (critT - ltp) * 0.5;
            rtp -= (rtp - critT) * 0.5;
         }
         // check for convergence
         if ( std::abs( rtp - ltp ) < m_epsT || std::abs( maxAoverB - minAoverB )/(maxAoverB + minAoverB) < m_eps )
         {
            m_AoverB = oldAoverB;
            return (maxAoverB + minAoverB) * 0.5;
         }
      }
      m_AoverB = oldAoverB; // failed to find for g_MaxStepsNum iterations
   }
   return m_AoverB;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Find critical point Tc and Pc in fastest way is possible
/// return found critical point (Tc,Pc) values if point was found or zero values if wasn't found
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::TPPoint PTDiagramCalculator::searchCriticalPoint()
{
   TPPoint critPt( 0.0, 0.0 );

   bool foundPt = true;

   TPLine isoLine;
   foundPt = traceContourLine( 0.5, isoLine, true );
   if ( foundPt )
   {
      critPt = TPPoint( isoLine.back().first, isoLine.back().second );
   }
   else // can't find 0.5 isoline, try another one
   {
      for ( double v = 0.9; !foundPt && v > 0.0; v -= 0.1 )
      {  
         if ( std::abs( v - 0.5 ) < m_eps ) continue; // ignore 0.5 isoline
         isoLine.clear();
         foundPt = traceContourLine( v, isoLine, true );
         if ( foundPt )
         {
            critPt = TPPoint( isoLine.back().first, isoLine.back().second );
         }
      }
   }
   return critPt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Set tolerance value which used for bisection iterations and for cutting very small phase fractions (<eps^2)
// tol new tolerance value
///////////////////////////////////////////////////////////////////////////////////////////////////
void PTDiagramCalculator::setTolValue( double tol )
{
   m_eps = tol;
   m_epsT = m_eps * (m_gridT.back() - m_gridT.front())/static_cast<double>(m_gridT.size()); // epsT = eps * dT
   m_epsP = m_eps * (m_gridP.back() - m_gridP.front())/static_cast<double>(m_gridP.size()); // epsP = eps * dP
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Get single phase separation line for 1 phase region as EosPack calculates it
// return separation line as set of points
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::TPLine PTDiagramCalculator::getSinglePhaseSeparationLine()
{
   // if done search before or there is no bubble/dew line just return m_spsLine
   if ( m_spsLine.size() ) return m_spsLine;

   int inEdge = -1;
   int p1 = static_cast<int>(m_gridP.size()) - 1;
   int t1 = 0;
   
   bool   foundBDPt = false;
   double foundP;
   double foundT;

   // run over upper border of P/T grid
   int phase1 = getPhase( p1, 0 );
   for ( int t = 1; t < m_gridT.size(); ++t )
   {
      int phase2 = getPhase( p1, t );
      if ( phase1 != phase2 && bothPhases != phase1 && bothPhases != phase2 ) //found
      {
         t1 = t-1;
         foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1, t1, p1, t1+1, foundP, foundT );
         --p1;
         inEdge = 1;
         break;
      }
      phase1 = phase2;
   }

   if ( inEdge < 0 )
   {
      p1 = 0;
      t1 = static_cast<int>(m_gridT.size()) - 1;
   
      int phase1 = getPhase( 0, t1 );
      // run over right border of P/T grid
      for ( int p = 1; p < m_gridP.size(); ++p )
      {
         int phase2 = getPhase( p, t1 );
         if ( phase1 != phase2 && bothPhases != phase1 && bothPhases != phase2 ) //found
         {
            p1 = p - 1;
            foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1, t1, p1+1, t1, foundP, foundT );
            --t1;
            inEdge = 2;
            break;
         }
         phase1 = phase2;
      }
   }

   if ( inEdge < 0 ) return m_spsLine; // doesn't find anything

   if ( foundBDPt ) m_spsLine.push_back( TPPoint( foundT, foundP ) );

   std::set<int>  trace; // for keeping cells through which line came across

   foundBDPt = true;
   while( foundBDPt )
   {
      if ( t1 + 1 == m_gridT.size() || p1 + 1 == m_gridP.size() ) { break; } //  can't trace outside grid

      int phase[4];
      phase[0] = getPhase( p1,   t1   );
      phase[1] = getPhase( p1+1, t1   );
      phase[2] = getPhase( p1+1, t1+1 );
      phase[3] = getPhase( p1,   t1+1 );

      bool inters[4] = { false, false, false, false };

      for ( int i = 0; i < 4; ++i )
      {
         if ( phase[i] != phase[(i+1)%4] )
         {
            // get vapour/liquid interface
            inters[i] = phase[i] != bothPhases && phase[(i+1)%4] != bothPhases ? true : false;
         }
      }

      foundBDPt = false;
      for ( int i = 0; i < 4; ++i )
      {
         if ( inters[i] && i != inEdge )
         {
            switch( i )
            {
               case 0: foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1,   t1,   p1+1, t1,   foundP, foundT ); break;
               case 1: foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1+1, t1,   p1+1, t1+1, foundP, foundT ); break;
               case 2: foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1,   t1+1, p1+1, t1+1, foundP, foundT ); break;
               case 3: foundBDPt = doBisectionForSinglePhaseSeparationLineSearch( p1,   t1,   p1,   t1+1, foundP, foundT ); break;
            }

            if ( foundBDPt )
            {
               m_spsLine.push_back( TPPoint( foundT, foundP ) );

               switch( i ) // choose next cell
               {
                  case 0: --t1; break;
                  case 1: ++p1; break;
                  case 2: ++t1; break;
                  case 3: --p1; break;
               }

               if ( t1 < 0 || p1 < 0 ) foundBDPt = false;

               // do checking is this cell was already in trace ? if yes - stop tracing
               foundBDPt = foundBDPt && trace.insert( p1 * 10000 + t1 ).second;

               inEdge = (i + 2) %4;
               break;
            }
         }
      }
   }
   // add the last point as point where liquid/vapour phases changed on bubble/dew line
   double oldCritT = m_critT;
   double oldCritP = m_critP;

   if ( findCriticalPointInChangePhaseAlongBubbleDewLine( false ) )
   {
      double foundT = m_critT;
      double foundP = m_critP;
      m_spsLine.push_back( TPPoint( foundT, foundP ) );

      m_critT = oldCritT;
      m_critP = oldCritP;
   }

   return m_spsLine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Extrapolate countour line last segment to one phase region and find phase change point on
// this extrapolated line (critical point)
//
// isoline array of points for countour line, used to estimate lenght of line for extrapolation 
// P1 first point of contour line interval pressure value
// P2 second point of contour line interval pressure value
// T1 first point of contour line interval temperature value
// T2 second point of contour line interval temperature value
//
// return true if phase change point was found, false otherwise
////////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::extrapolateContourLineToOnePhaseRegion( const TPLine & isoline, double P1, double T1, double P2, double T2, double & critP, double & critT )
{
   // get extrapolated point on grid boundary
   double P3 = -1;
   double T3 = -1;

   bool found = false;
   // scale point to [0:1,0:1] 
   double scaleT = 1.0 / (m_gridT.back() - m_gridT.front());
   double scaleP = 1.0 / (m_gridP.back() - m_gridP.front());

   double isolLen = 0;
   for ( size_t i = 1; i < isoline.size(); ++i )
   {
      double vx = ( isoline[i].first  - isoline[i-1].first  ) * scaleT;
      double vy = ( isoline[i].second - isoline[i-1].second ) * scaleP;
      isolLen += sqrt( vx * vx + vy * vy );
   }

   double x1 = (T1 - m_gridT.front() ) * scaleT;
   double y1 = (P1 - m_gridP.front() ) * scaleP;
   double x2 = (T2 - m_gridT.front() ) * scaleT;
   double y2 = (P2 - m_gridP.front() ) * scaleP;

   for ( int i = 0; !found && i < 4; ++i )
   {
      switch( i )
      {
         case 0: // upper boundary
            if ( std::abs( P2 - P1 ) > m_eps )
            {
               P3 = m_gridP.back();
               T3 = T1 + (P3 - P1) / (P2 - P1) * (T2 - T1);
            }
            break;

         case 1: // right grid boundary
            if ( std::abs( T2 - T1 ) > m_eps )
            {
               T3 = m_gridT.back();
               P3 = P1 + (T3 - T1)/(T2 - T1) * (P2 - P1);
            }
            break;

         case 2: // left grid boundary
            if ( std::abs( T2 - T1 ) > m_epsT )
            {
               T3 = m_gridT.front();
               P3 = P1 + (T3 - T1)/(T2 - T1) * (P2 - P1);
            }
            break;

         case 3: // down boundary
            if ( std::abs( P2 - P1 ) > m_epsP )
            {
               P3 = m_gridP.front();
               T3 = T1 + (P3 - P1) / (P2 - P1) * (T2 - T1);
            }
            break;
      }
      double x3 = (T3 - m_gridT.front() ) * scaleT;
      double y3 = (P3 - m_gridP.front() ) * scaleP;

      if ( ((x2-x1)*(x3-x1) + (y2-y1)*(y3-y1)) >= 0.0 &&
           (T3+m_epsT) > m_gridT.front() && (T3-m_epsT) < m_gridT.back() &&
           (P3+m_epsP) > m_gridP.front() && (P3-m_epsP) < m_gridP.back() )
      {
         found = true;
      }
   }
   if ( T3 < m_gridT.front() || T3 > m_gridT.back() || P3 < m_gridP.front() || P3 > m_gridP.back() ) return false;

   double phaseFrac[2];

   int phase3 = getMassFractions( P3, T3, m_masses, phaseFrac );
   ++m_isoBisecIters;

   // left boundary for the iteration along line
   double LP = P2;
   double LT = T2;

   bool foundPt = false; //not found yet;
   if ( bothPhases != phase3 )
   {
      // ok, P3,T3 now outside of 2 phase region, find exact point by doing bisections for P & T along line
      // extrapolating last segment of contour line
      for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
      {
         double newP = 0.5 * (LP + P3);
         double newT = 0.5 * (LT + T3);

         int newPhase = getMassFractions( newP, newT, m_masses, phaseFrac );
         ++m_isoBisecIters;

         if ( bothPhases == newPhase )
         {
            LP = newP;
            LT = newT;
         }
         else
         {
            P3 = newP;
            T3 = newT;
         }

         x1 = (LT - m_gridT.front() ) * scaleT;
         y1 = (LP - m_gridP.front() ) * scaleP;
         x2 = (T3 - m_gridT.front() ) * scaleT;
         y2 = (P3 - m_gridP.front() ) * scaleP;
 
         // check convergence
         if ( sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) ) < m_eps ) // check convergence in [0:1,0:1] plane
         {
            x1 = (T2 - m_gridT.front() ) * scaleT;
            y1 = (P2 - m_gridP.front() ) * scaleP;
            if ( sqrt( (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) ) < 0.2 * isolLen ) // is it too far for extrapolation (crossed other side of 2 phase region)?
            { 
               critT = T3;
               critP = P3;
               foundPt = true;
            }
            break;
         }
      }
   }
   return foundPt;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Get cricondentherm point for defined in constructor composition. Must be called after bubble/dew lines calculation
// return point on bubble/dew curve with maximum temperature
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::TPPoint PTDiagramCalculator::getCricondenthermPoint() const
{
   TPPoint ret( 0.0, 0.0 );
   
   TPLine::const_iterator cct = m_bubbleDewLine.begin();

   for ( TPLine::const_iterator it = m_bubbleDewLine.begin(); it != m_bubbleDewLine.end(); ++it )
   {
      if ( cct->first < it->first ) { cct = it; ret = *it; }
   }
   return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Get cricondenbar point for defined in constructor composition. Must be called after bubble/dew lines calculation
// return point on bubble/dew curve with maximum pressure
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::TPPoint PTDiagramCalculator::getCricondenbarPoint() const
{
   TPPoint ret( 0.0, 0.0 );

   TPLine::const_iterator cct = m_bubbleDewLine.begin();

   for ( TPLine::const_iterator it = m_bubbleDewLine.begin(); it != m_bubbleDewLine.end(); ++it )
   {
      if ( cct->second < it->second ) { cct = it; ret = *it; }
   }
   return ret;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// calculate bubble pressure for given Temperature
// T given temperature value 
// bubbleP - on success it contains bubble pressure for given temperature
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::getBubblePressure( double T, double * bubbleP ) const
{
   assert( bubbleP != NULL );
   
   for ( int i = static_cast<int>(m_bubbleDewLine.size())-2; i >= 0 ; --i )
   {
      if ( std::min( m_bubbleDewLine[i].first, m_bubbleDewLine[i+1].first ) <  T && 
           std::max( m_bubbleDewLine[i].first, m_bubbleDewLine[i+1].first ) >= T )
      {
         double minT = m_bubbleDewLine[i].first;
         double minP = m_bubbleDewLine[i].second;
         double maxT = m_bubbleDewLine[i+1].first;
         double maxP = m_bubbleDewLine[i+1].second;
         
         *bubbleP = minP + (T - minT) / (maxT - minT) * (maxP - minP);
         return true;
      }
   }
   return false;
}
   

///////////////////////////////////////////////////////////////////////////////////////////////////
// Return which phase exist for given (p,t)
// p pressure
// t temperature
// return phase ID
///////////////////////////////////////////////////////////////////////////////////////////////////
int PTDiagramCalculator::getPhase( size_t p, size_t t )
{
   assert( p < m_gridP.size() && t < m_gridT.size() );

   int phase = unknown;

   if ( m_liqFrac[p][t] < 0.0 )
   {
      double phaseFrac[2];
      phase = getMassFractions( m_gridP[p], m_gridT[t], m_masses, phaseFrac );
      ++m_bdBisecIters;

      m_liqFrac[p][t] = phaseFrac[PhaseId::LIQUID];
   }
   else
   {
      phase = (m_liqFrac[p][t]  > 0.0 ? liquidPhase : unknown) + ((1.0 - m_liqFrac[p][t]) > 0.0 ? vaporPhase : unknown);
   }

   return phase;
}

