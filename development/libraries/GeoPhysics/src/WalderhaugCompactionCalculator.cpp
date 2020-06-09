/**
 * \file WalderhaugCompactionCalculator.cpp
 * \brief WalderhaugCompactionCalculator derived class which computes quartz cementation on a basin with Walderhaug model
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 * 
 * The chemical compaction is processed on each valid node of the whole basin.
 * The abstract class is described in ChemicalCompactionCalculator.h.
 * The equation used is described in [Walderhaug, 1996]
 */

#include "WalderhaugCompactionCalculator.h"
#include "GeoPhysicalConstants.h"

// std library
#include <cmath>
#include <cassert>
#include <algorithm>

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MillionYearToSecond;

namespace GeoPhysics{

const double WalderhaugCompactionCalculator::m_coefA = 1.98e-22;       //[mol/s/cm2]
const double WalderhaugCompactionCalculator::m_coefB = 0.022;          //[1/degree Celsius]
const double WalderhaugCompactionCalculator::m_startTemperature = 80.0 ; //[degree Celsius]
// m_limitTemp = (log(1.7)+308*M_LN10) / (m_coefB * M_LN10);
// Approximation at 14010 celsius degree
const double WalderhaugCompactionCalculator::m_limitTemp = 14010.0;
  

WalderhaugCompactionCalculator::WalderhaugCompactionCalculator() :
  ChemicalCompactionCalculator(),
  m_constantCoef( MolarMassQuartz * m_coefA / DensityQuartz )
{
   //----------Computation of the constant factor of the equation----------------------
   //constantCoef = M*a/V/rho
   //with M the molar mass of quartz [g/mol], a a constant coefficient [mol/s/cm2], V the unit volume = 1 [cm3], rho the density of quartz [g/cm3]
}

WalderhaugCompactionCalculator::~WalderhaugCompactionCalculator()
{
}

void WalderhaugCompactionCalculator::computeOnTimeStep( Grid & grid )
{

   setLithologies( grid );

   // Initialize the initial porosity
   if ( m_initialPorosity.size() == 0 )
   {
      const int size = grid.getSize();
      m_initialPorosity.resize( size, -1.0 );
   }

   // Store the needed data
   const double * previousTemperatureGrid        = grid.getPreviousTemperature();
   double * chemicalCompactionGrid               = grid.setChemicalCompaction();
   const double * currentTemperatureGrid         = grid.getCurrentTemperature();
   const double * currentPorosityGrid            = grid.getPorosity();

   // Compute the time step
   const double timeStep = -(grid.getCurrentTime() - grid.getPreviousTime());
   assert( ("Time step cannot be negative", timeStep >= 0) );

   // Computation should only be done on active nodes ie the nodes of the valid needles of the compactable layers
      // which are part of the below the sea bottom
      const int numberOfActiveNodes = grid.getNumberOfActiveNodes();
      const int * activeNodes       = grid.getActiveNodes();
      
   // Path through all active nodes
   for ( int activeNodeIndex = 0; activeNodeIndex < numberOfActiveNodes; ++activeNodeIndex )
   {
      const int refNode = activeNodes[activeNodeIndex];

      // Access the corresponding Lithology and store the parameters which will be used
      const int lithoID = m_lithologyMap[refNode];

      // Get the previous temperature in [degree Celsius] for the formula
      double currentTemperature = currentTemperatureGrid[refNode];// [degree Celsius]
      if( currentTemperature > m_limitTemp )
      {
   currentTemperature = m_limitTemp;
      }

      //Cementation doesn't start before m_startTemperature = 80 degree Celsius
      if ( currentTemperature > m_startTemperature )
      {
         // Get the previous temperature and chemical comapction and the current porosity
         double previousTemperature              = previousTemperatureGrid[refNode];                   // [degree Celsius]
    if( previousTemperature > m_limitTemp )    
    {
       previousTemperature = m_limitTemp;
    }
         const double chemicalCompaction         = chemicalCompactionGrid[refNode];                    //[fraction of the unit volume]
         const double currentPorosity            = currentPorosityGrid[refNode];                       //[fraction of the unit volume] 
        
         // If it is the first time step where compaction occurs for this node, store the initial porosity
         if ( m_initialPorosity[refNode] < 0 )
         {
           assert( ( "The porosity cannot be zero", currentPorosity != 0) );
            m_initialPorosity[refNode] = currentPorosity;
         }
         const double initialPorosity = m_initialPorosity[refNode];

         // Computation of the cemented fraction of the unit volume step by step
         //quartzSurface = 6 * f * V * ( 1 - C ) / D * porosity / initialporosity [cm2]
         //with f the proportion of quartz in the rock, V the unit volume = 1 [cm3], C the coating factor, D the size of grains [cm]
         double cementedFraction = m_lithologyList[lithoID] * currentPorosity / initialPorosity;

         // 2todo use log constant
         static const double ln10 = std::log( 10.0 );
         //If the temperatures are very close or equal, in order to avoid to divide by a nul number
         if ( std::fabs( currentTemperature - previousTemperature ) < 1e-10 )
         {
            cementedFraction *= m_constantCoef * timeStep * MillionYearToSecond * exp( m_coefB * currentTemperature * ln10 );
         }
         else
         {
            cementedFraction *= m_constantCoef * timeStep * MillionYearToSecond;
            cementedFraction *= ( exp( m_coefB * currentTemperature * ln10 ) - exp( m_coefB * previousTemperature * ln10 ) );
            cementedFraction /= m_coefB * ln10 * ( currentTemperature - previousTemperature );
         }
         assert( ("The cemented fraction is not a number, one of the parameters is wrong.>", !std::isnan( cementedFraction )) );
         
         //Store the new value in the grid
         chemicalCompactionGrid[refNode] = std::max( chemicalCompaction - cementedFraction, -1.0 );
      }
   } // end of the path through active nodes
}

void WalderhaugCompactionCalculator::setLithologies( const Grid & grid )
{
   //Store the lithology parameters in structures and store the structures in a vector
   const double *  grainSize     = grid.getQuartzGrainSize();
   const double * fractionQuartz = grid.getQuartzFraction();
   const double * coatingClay    = grid.getClayCoatingFactor();
   const int numberLithologies   = grid.getNumberOfLithologies();
   m_lithologyList.resize( numberLithologies );

   for ( int lithoIndex = 0 ; lithoIndex <numberLithologies ; ++lithoIndex )
   { 
      assert( ("Quartz need stricly positive grain size", grainSize[lithoIndex] > 0) );
      m_lithologyList[lithoIndex] = ( 1 - coatingClay[lithoIndex] ) * 6 * fractionQuartz[lithoIndex] / grainSize[lithoIndex];
   }
   //Store the lithology of each node
   grid.getLithologyMap( m_lithologyMap );
}

};//end of namespace
