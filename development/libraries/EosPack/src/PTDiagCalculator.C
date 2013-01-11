// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include "EosPack.h"
#include "PTDiagCalculator.h"
#include "consts.h"

#include <iostream>
#include <cassert>
#include <cmath>
#include <fstream>
#include <memory>
#include <numeric>
#include <set>

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
static const double g_MaximalPressure    = 30e6;     // default maximum Pressure value for diagram 15 MPa
static const double g_MaximalDiagramPressure = g_MaximalPressure * 3; // absolute maximum pressure for diagram. Sometimes it could be more than 300 MPa 
static const int    g_GridSize           = 100;      // number of points between Min/Max temperature and pressure
static const double g_Tolerance          = 1e-4;     // stop tolerance for bisection iterations
static const int    g_MaxStepsNum        = 200;      // maximum steps number in bisections iterations
static const int    g_GridExtStep        = 5;        // extend T grid when needed for this number of points


///////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Call PVT library and calculate mole/volume or mass fraction for Liquid/Vapor phases for given composition for single set of P, T values
/// \param p pressure
/// \param t temperature
/// \param composition  array of mass values for each component
/// \param[out] massFraction on return contains accumulated fractions for vapor/liquid phases
/// \param massFracType which fraction should be calculated - mass, mole or volume fraction
/// \return which phases exist in given composition for given P and T. "unknown" if call for flashing was failed
///////////////////////////////////////////////////////////////////////////////////////////////////
static int GetMassFractions( double p, double t, const std::vector<double> & composition, double massFraction[2], PTDiagramCalculator::DiagramType & massFracType )
{
   // Get acronims of some constants
   const int iNc     = CBMGenerics::ComponentManager::NumberOfSpecies;
   const int iNp     = CBMGenerics::ComponentManager::NumberOfPhases;
   const int iLiquid = CBMGenerics::ComponentManager::Liquid;
   const int iVapour = CBMGenerics::ComponentManager::Vapour;
   
   // arrays for passing to flasher
   double masses[iNc];
   double phaseMasses[iNp][iNc];
   double phaseDensities[iNp];

   assert( composition.size() == iNc );
   std::copy( composition.begin(), composition.begin() + iNc, &(masses[0]) );

   std::fill_n( &(phaseMasses[0][0]), sizeof( phaseMasses )    / sizeof( double ), 0.0 );
   std::fill_n( phaseDensities,       sizeof( phaseDensities ) / sizeof( double ), 0.0 );


   // change the default behaviour for labeling phases for high temperature span
   pvtFlash::EosPack::getInstance().setCritAoverBterm( 2.0 ); 

   // Call flasher to get compositions for phases
   bool res = pvtFlash::EosPack::getInstance().computeWithLumping( t, p, masses, phaseMasses, phaseDensities, NULL );  

   // revert back flasher settings
   pvtFlash::EosPack::getInstance().resetToDefaultCritAoverBterm();

   if ( !res ) return unknown;

   double totMass = std::accumulate( masses, masses + iNc, 0.0 ); // collect total mass for normalisation
   assert( totMass > 0.0 );

   double total = 0.0;

   switch( massFracType )
   {
      case PTDiagramCalculator::MoleMassFractionDiagram: // convert to molar mass fraction
         for ( int phase = 0; phase < iNp; ++phase )
         {
            for ( int comp = 0; comp < iNc; ++comp )
            {
               phaseMasses[phase][comp] /= totMass * CBMGenerics::ComponentManager::MolecularWeight[comp];
               total += phaseMasses[phase][comp];
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

   // normalise fractions
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
// Constructor. It also creates default grid for T from g_MinimalTemperature:g_MinimalTemperature
// typeOfDiagram which type of diagram should be built - Mass/Mole/Volume fraction
// massFraction array of component masses
///////////////////////////////////////////////////////////////////////////////////////////////////
PTDiagramCalculator::PTDiagramCalculator( DiagramType & typeOfDiagram, const std::vector<double> & massFraction ) : 
      m_eps( g_Tolerance ),
      m_critP( -1.0 ),
      m_critT( -1.0 ),
      m_bdBisecIters( 0 ),
      m_isoBisecIters( 0 )
{
   m_diagType = typeOfDiagram;
   m_masses   = massFraction;

   assert( m_masses.size() == CBMGenerics::ComponentManager::NumberOfSpecies );

   // create default grid for temperature and pressure
   generatePTGrid( g_MinimalPressure, g_MaximalPressure, g_MinimalTemperature, g_MaximalTemperature );
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
   size_t newSize  = oldSize + std::floor( ( newPMax - m_gridP.back() ) / presStep + 0.5 );

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

      int phase = GetMassFractions( (bisectT ? foundP : V), (bisectT ? V : foundT), m_masses, phaseFrac, m_diagType ); // get phase composition for new guess
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
bool PTDiagramCalculator::doBisectionForContourLineSearch( size_t p1, size_t t1, size_t p2, size_t t2, double frac, double & foundP, double & foundT )
{
   assert( p1 < m_gridP.size() && p2 < m_gridP.size() && t1 < m_gridT.size() && t2 < m_gridT.size() );

   const int iLiquid = CBMGenerics::ComponentManager::Liquid;

   int phase1 = getPhase( p1, t1 );
   int phase2 = getPhase( p2, t2 );
 
   // flasher failed for some reason, can't calculate bubble/dew point, or we do not have phase transition for (P1,T1)->(P2,T2)
   if ( unknown == phase1 || unknown == phase2 ) { return false; } 

   double frac1 = m_liqFrac[p1][t1];
   double frac2 = m_liqFrac[p2][t2];

   // contour line comes outside of given segment
   if ( std::min( frac1, frac2 ) > frac || std::max( frac1, frac2 ) < frac || (phase1 == phase2 && bothPhases != phase1) )
   { 
      if ( std::abs( frac - frac1 ) < std::abs( frac - frac2 ) ) // chose nearest point
      {
         foundT = m_gridT[t1];
         foundP = m_gridP[p1];
      }
      else
      {
         foundT = m_gridT[t2];
         foundP = m_gridP[p2];
      }
      return false;
   }

   bool bisectT = p1 == p2 && t1 != t2 ? true : false;
   if ( !bisectT && (p1 == p2 || t1 != t2) ) assert(0);
 
   double V1 = bisectT ? m_gridT[t1] : m_gridP[p1];
   double V2 = bisectT ? m_gridT[t2] : m_gridP[p2];

   foundT = m_gridT[t1];
   foundP = m_gridP[p1];
 
   for ( int steps = 0; steps < g_MaxStepsNum && std::abs( V1 - V2 ) > m_eps; ++steps )
   {            
      double phaseFrac[2];

      // make the new guess
      double V = V1 + (frac - frac1)/(frac2 - frac1) * (V2 - V1);

      int phase = GetMassFractions( (bisectT ? foundP : V), (bisectT ? V : foundT), m_masses, phaseFrac, m_diagType );
      ++m_isoBisecIters;

      if ( bothPhases != phase ) // outside of the bubble/dew curve
      {
         phaseFrac[iLiquid] = phase1 == phase ? 0.0 : 1.0;
      }

      if ( phaseFrac[iLiquid] < frac )
      {
         V1 = V;
         frac1 = phaseFrac[iLiquid]; 
         phase1 = phase;
      }
      else if ( phaseFrac[iLiquid] > frac )
      {
         V2 = V;
         frac2 = phaseFrac[iLiquid];
         phase2 = phase;
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

   const int iLiquid = CBMGenerics::ComponentManager::Liquid;
   const int iVapour = CBMGenerics::ComponentManager::Vapour;

   double phaseFrac[2];

   // starting tracing Bubble/Dew point curve
   double foundT;
   double foundP;

   int    t1 = -1;
   int    p1 = -1;
   int    inEdge = -1;

   // as first step do fraction calculation on the grid T and P till we'll find bubble/dew line starting point
   for ( size_t pi = 0; pi < m_gridP.size()-1 && p1 < 0; ++pi )
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
               m_bubbleDewLine.push_back( std::pair<double,double>( foundT, foundP ) );
            }
         }
         else if ( getPhase( pi, ti ) != getPhase( pi, ti+1 ) )
         {
            if ( doBisectionForBubbleDewSearch( pi, ti, pi, ti+1, foundP, foundT ) )
            {
               t1 = ti;
               p1 = pi;
               inEdge = 3;
               m_bubbleDewLine.push_back( std::pair<double,double>( foundT, foundP ) );
            }
         }
      }
   }

   // scanned all grid by can't find bubble/dew line - can't build diagram
   if ( t1 < 0 && p1 < 0 ) { return; }

   std::set<int>  trace; // for keeping cells through which isoline came across

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
                  m_bubbleDewLine.push_back( std::pair<double,double>( foundT, foundP ) );

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

   // trace 0.5 contour line at first
   m_c0p5Line = calcContourLine( 0.5 );

   findCriticalPoint();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Find critical point as intersection of 0.5 isoline with bubble/dew curve. If not found - go along bubble/dew curve
//  and look for phase changed from gas to liquid
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::findCriticalPoint()
{
   bool foundCriticalPoint = false;

   m_critPointPos = m_bubbleDewLine.end();

   double scaleT = 1.0 / (m_gridT.back() - m_gridT.front());
   double scaleP = 1.0 / (m_gridP.back() - m_gridP.front());

   if ( m_c0p5Line.size() > 3 ) // find critical point on to Bubble/Dew curve
   {  double minDist = 2;

      // convert to 0-1 segment
      double critT = ( m_c0p5Line.back().first  - m_gridT.front() ) * scaleT;
      double critP = ( m_c0p5Line.back().second - m_gridP.front() ) * scaleP;

      for ( std::vector< std::pair<double,double> >::iterator it = m_bubbleDewLine.begin(); it != m_bubbleDewLine.end(); ++it )
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
         if ( std::abs( m_critPointPos->first  - m_c0p5Line.back().first  ) < ( m_gridT[1] - m_gridT[0] ) * 0.5 &&
              std::abs( m_critPointPos->second - m_c0p5Line.back().second ) < ( m_gridP[1] - m_gridP[0] ) * 0.5 
            )
         {
            // and also it is not already on bubble dew curve
            if ( (( m_critPointPos->first  - m_c0p5Line.back().first ) > m_eps) || (( m_critPointPos->second - m_c0p5Line.back().second ) > m_eps) )
            {
               m_critT = m_c0p5Line.back().first;
               m_critP = m_c0p5Line.back().second;
               m_critPointPos = m_bubbleDewLine.insert( (m_critPointPos->first > m_critT ? m_critPointPos : (m_critPointPos + 1)), 
                                                     std::pair<double, double>( m_critT, m_critP ) );
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

   // if critical point wasn't found, try to find critical point by tracking changing phases from vapor to liquid along buble/dew line
   if ( !foundCriticalPoint && m_bubbleDewLine.size() > 2 )
   {
      double massFraction[2];
      int phase1 = GetMassFractions( m_bubbleDewLine.front().second, m_bubbleDewLine.front().first, m_masses, massFraction, m_diagType );
      ++m_bdBisecIters;

      m_critPointPos = m_bubbleDewLine.begin();
      for ( std::vector< std::pair<double,double> >::iterator it = m_critPointPos + 1; it != m_bubbleDewLine.end() && !foundCriticalPoint; ++it )
      {
         int phase2 = GetMassFractions( it->second, it->first, m_masses, massFraction, m_diagType );
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

               int newPhase = GetMassFractions( newP, newT, m_masses, massFraction, m_diagType );
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

                  m_critPointPos = m_bubbleDewLine.insert( it, std::pair<double, double>( m_critT, m_critP ) );
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
   }
   return foundCriticalPoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Calculate contour line for given value. Should be called after findBubleDewLines().
// val Liquid fraction value for contour line. Must be between 0 and 1
// returns array of (T,P) pair for each contour line point
///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector< std::pair<double, double> > PTDiagramCalculator::calcContourLine( double val )
{
   std::vector< std::pair<double,double> > isoline;

   if ( std::abs( val - 1.0 ) < m_eps ) // request for bubble curve
   {
      for ( int ti = m_bubbleDewLine.size() - 1; ti >= (m_critPointPos - m_bubbleDewLine.begin()); --ti )
      {
         isoline.push_back( m_bubbleDewLine[ti] );
      }
      return isoline;
   }
   
   if ( val < m_eps ) // request for dew curve
   {
      for ( int ti = (m_critPointPos - m_bubbleDewLine.begin()); ti >= 0; --ti )
      {
         isoline.push_back( m_bubbleDewLine[ti] );
      }
      return isoline;
   }

   if ( std::abs( val - 0.5 ) < m_eps && m_c0p5Line.size() ) // request for 0.5 contour line
   {
      return m_c0p5Line;
   }

   // find starting point
   int t1     = -1;
   int p1     = -1;
   int inEdge = -1;

   double foundT;
   double foundP;

   for ( size_t pi = 0; pi < m_gridP.size()-1 && p1 < 0; ++pi )
   {
      for ( size_t ti = 0; ti < m_gridT.size()-1 && t1 < 0; ++ti )
      {
         int phase[2];
         phase[0] = getPhase( pi,   ti );
         phase[1] = getPhase( pi+1, ti );

         if ( bothPhases == phase[0] && bothPhases == phase[1] &&
              std::min( m_liqFrac[pi][ti], m_liqFrac[pi+1][ti] ) < val && val <= std::max( m_liqFrac[pi][ti], m_liqFrac[pi+1][ti] ) )
         {
            if ( doBisectionForContourLineSearch( pi, ti, pi+1, ti, val, foundP, foundT ) )
            {
               t1 = ti;
               p1 = pi;
               inEdge = 0;
               isoline.push_back( std::pair<double,double>( foundT, foundP ) );
            }
         }
         else
         {
            phase[1] = getPhase( pi, ti+1 );
            if ( bothPhases == phase[0] && bothPhases == phase[1] &&
                 std::min( m_liqFrac[pi][ti], m_liqFrac[pi][ti+1] ) < val && val <= std::max( m_liqFrac[pi][ti], m_liqFrac[pi][ti+1] ) )
            {
               if ( doBisectionForContourLineSearch( pi, ti, pi, ti+1, val, foundP, foundT ) )
               {
                  t1 = ti;
                  p1 = pi;
                  inEdge = 3;
                  isoline.push_back( std::pair<double,double>( foundT, foundP ) );
               }
            }
         }
         if ( bothPhases == phase[0] && bothPhases != getPhase( pi, ti+1 ) ) break;
      }
   }
   // scanned all grid by can't find bubble/dew line - can't build diagram
   if ( t1 < 0 && p1 < 0 ) { return isoline; }

   std::set<int>  trace; // for keeping cells through which isoline came across

   // now we can trace bubble-dew line
   bool foundPt = true;
   while( foundPt )
   {
      if ( t1 + 1 == m_gridT.size() || p1 + 1 == m_gridP.size() ) { break; } //  can't trace outside grid
      
      int phase[4];
      // check that all values for cell corners are exist
      phase[0] = getPhase( p1,   t1   );
      phase[1] = getPhase( p1+1, t1   );
      phase[2] = getPhase( p1+1, t1+1 );
      phase[3] = getPhase( p1,   t1+1 );
      
      double fracVals[4];
      fracVals[0] = m_liqFrac[p1  ][t1  ]; 
      fracVals[1] = m_liqFrac[p1+1][t1  ];
      fracVals[2] = m_liqFrac[p1+1][t1+1];
      fracVals[3] = m_liqFrac[p1  ][t1+1];

      bool inters[4] = { false, false, false, false };

      for ( int i = 0; i < 4; ++i )
      {
         if ( !inters[i] && std::min( fracVals[i], fracVals[(i+1)%4] ) < val && val < std::max( fracVals[i], fracVals[(i+1)%4] ) )
         {
            inters[i] = true;
         }
      }
      //assert( inters[inEdge] );

      foundPt = false;

      for ( int i = 0; i < 4; ++i )
      {
         // try all intersected edges other than inEdge to find out point
         if ( inters[i] && i != inEdge )
         {
            switch( i )
            {
               case 0: foundPt = doBisectionForContourLineSearch( p1,   t1,   p1+1, t1,   val, foundP, foundT ); break;
               case 1: foundPt = doBisectionForContourLineSearch( p1+1, t1,   p1+1, t1+1, val, foundP, foundT ); break;
               case 2: foundPt = doBisectionForContourLineSearch( p1,   t1+1, p1+1, t1+1, val, foundP, foundT ); break;
               case 3: foundPt = doBisectionForContourLineSearch( p1,   t1,   p1,   t1+1, val, foundP, foundT ); break;
            }

            if ( foundPt )
            {  
               if ( isoline.size() > 2 ) 
               {
                  // check that angle between new segment and previous segment more than Pi/2. Contour line should be smooth
                  // if angle is less then Pi/2 - good chance that we near the end of contour line
                  size_t sz = isoline.size();
                  // go to 0-1,0-1 coordinates
                  double P1 = ( isoline[sz - 2].second - m_gridP.front() ) / ( m_gridP.back() - m_gridP.front() );
                  double T1 = ( isoline[sz - 2].first  - m_gridT.front() ) / ( m_gridT.back() - m_gridT.front() );

                  double P2 = ( isoline[sz - 1].second - m_gridP.front() ) / ( m_gridP.back() - m_gridP.front() );
                  double T2 = ( isoline[sz - 1].first  - m_gridT.front() ) / ( m_gridT.back() - m_gridT.front() );

                  double P3 = ( foundP - m_gridP.front() ) / ( m_gridP.back() - m_gridP.front() );
                  double T3 = ( foundT - m_gridT.front() ) / ( m_gridT.back() - m_gridT.front() );

                  double cosa = ((P2 - P1) * (P3 - P2) + (T2 - T1) * (T3 - T2)) / sqrt( (P2 - P1) * (P2 - P1) + (T2 - T1) * (T2 - T1) ) 
                                                                                / sqrt( (P3 - P2) * (P3 - P2) + (T3 - T2) * (T3 - T2) );
                  foundPt = cosa < -m_eps ? false : true;
               }
               
               if ( foundPt ) isoline.push_back( std::pair<double,double>( foundT, foundP ) );

               switch( i ) // choose next cell
               {
                  case 0: --t1; break;
                  case 1: ++p1; break;
                  case 2: ++t1; break;
                  case 3: --p1; break;
               }
               if ( t1 < 0 || p1 < 0 ) foundPt = false;

               // do checking is this cell was already in trace ? if yes - stop tracing
               foundPt = foundPt && trace.insert( p1 * 10000 + t1 ).second ;

               inEdge = (i + 2) %4;
               break;
            }
         }
      }
   }

   // finding critical point by finding bubble/dew point on extrapolation of contour line
   if ( isoline.size() > 3 ) 
   {
      size_t lastPt = isoline.size() - 1;

      double P1 = isoline[lastPt-1].second;
      double T1 = isoline[lastPt-1].first;

      double P2 = isoline[lastPt].second;
      double T2 = isoline[lastPt].first;

      double P3 = P2;
      double T3 = T2;

      // make an attempt to go out of 2 phase region
      int phase3 = bothPhases;
      
      double P = P2;
      double T = T2;

      double phaseFrac[2];
      
      double dP = m_gridP[1] - m_gridP[0];
      double dT = m_gridT[1] - m_gridT[0];

      if ( std::abs( P2 - P1 ) / dP < std::abs( T2 - T1 ) / dT )
      {
         dT *= T1 > T2 ? -1 : 1;
         dP = (P2 - P1 ) / (T2 - T1) * dT;
      }
      else
      {
         dP *= P1 > P2 ? -1 : 1;
         dT = (T2 - T1 ) / (P2 - P1) * dP;
      }

      // do not make extrapolations more than 3 cells
      for ( int steps = 0; phase3 == bothPhases && steps < 3; ++steps )
      {
         P3 += dP;
         T3 += dT;

         // check if we still inside grid
         if ( P3 < m_gridP.front() || P3 > m_gridP.back() || T3 < m_gridT.front() || T3 > m_gridT.back()  ) break;

         double phaseFrac[2];
         phase3 = GetMassFractions( P3, T3, m_masses, phaseFrac, m_diagType );
         ++m_isoBisecIters;
         
         if ( bothPhases == phase3 ) // keep last point where we have 2 phase region
         {
            P = P3;
            T = T3;
         }
      }

      if ( bothPhases != phase3 )
      {
         // ok, P3,T3 now outside of 2 phase region, find exact point by doing bisections for P & T along line
         // extrapolating last segment of contour line
         for ( int steps = 0; steps < g_MaxStepsNum; ++steps )
         {
            double newP = 0.5 * (P + P3);
            double newT = 0.5 * (T + T3);

            int newPhase = GetMassFractions( newP, newT, m_masses, phaseFrac, m_diagType );
            ++m_isoBisecIters;

            if ( bothPhases == newPhase )
            {
               P = newP;
               T = newT;
            }
            else
            {
               P3 = newP;
               T3 = newT;
            }
            // check convergence
            if ( std::max( std::abs( P - P3 ) / ( P + P3 ), std::abs( T - T3 ) / ( T + T3 ) ) < m_eps )
            {
               isoline.push_back( std::pair<double,double>( T3, P3 ) );
               break;
            }
         }
      }
   }
   return isoline;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// calculate bubble pressure for given Temperature
// T given temperature value 
// bubbleP - on success it contains bubble pressure for given temperature
// return true on success, false otherwise
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PTDiagramCalculator::getBubblePressure( double T, double & bubbleP ) const
{
   for ( int i = m_bubbleDewLine.size()-2; i >= 0 ; --i )
   {
      if ( std::min( m_bubbleDewLine[i].first, m_bubbleDewLine[i+1].first ) <  T && 
           std::max( m_bubbleDewLine[i].first, m_bubbleDewLine[i+1].first ) >= T )
      {
         double minT = m_bubbleDewLine[i].first;
         double minP = m_bubbleDewLine[i].second;
         double maxT = m_bubbleDewLine[i+1].first;
         double maxP = m_bubbleDewLine[i+1].second;
         
         bubbleP = minP + (T - minT) / (maxT - minT) * (maxP - minP);
         return true;
      }
   }
   return false;
}
   

int PTDiagramCalculator::getPhase( size_t p, size_t t )
{
   assert( p < m_gridP.size() && t < m_gridT.size() );

   int phase = unknown;

   if ( m_liqFrac[p][t] < 0.0 )
   {
      double phaseFrac[2];
      phase = GetMassFractions( m_gridP[p], m_gridT[t], m_masses, phaseFrac, m_diagType );
      ++m_bdBisecIters;

      m_liqFrac[p][t] = phaseFrac[CBMGenerics::ComponentManager::Liquid];
   }
   else
   {
      phase = (m_liqFrac[p][t]  > 0.0 ? liquidPhase : unknown) + ((1.0 - m_liqFrac[p][t]) > 0.0 ? vaporPhase : unknown);
   }

   return phase;
}
