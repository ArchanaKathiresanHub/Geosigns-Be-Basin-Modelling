/**
 * \file SchneiderCompactionCalculator.C
 * \brief SchneiderCompactionCalculator class which computes quartz cementation on a basin with Schneider model
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 * The chemical compaction is process on each valid node of the whole basin
 * The abstract class is in ChemicalCompactionCalculator.h
 * The equation used is described in [Schneider, 1996]
 * 
 */

#include "SchneiderCompactionCalculator.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include "GeoPhysicalConstants.h"

namespace GeoPhysics{

SchneiderCompactionCalculator :: ~SchneiderCompactionCalculator()
{

}

void SchneiderCompactionCalculator::computeOnTimeStep( Grid & grid )
{
	setLithologies( grid );

	// Store the needed data
	const double * currentTemperatureGrid         = grid.getCurrentTemperature();
	const double * currentPorosityGrid            = grid.getPorosity();
	const double * currentVESGrid                 = grid.getVES();
	double * chemicalCompactionGrid               = grid.setChemicalCompaction();

	// Compute the time step
	const double timeStep = grid.getPreviousTime() - grid.getCurrentTime();
	assert( ("Time step cannot be negative", timeStep >= 0) );

	// Computation should only be done on active nodes ie the nodes of the valid needles of the compactable layers
	// which are part of the below the sea bottom
	const int numberOfActiveNodes = grid.getNumberOfActiveNodes();
	const int * activeNodes       = grid.getActiveNodes();

	// Path through all active nodes
	for ( int activeNodeIndex = 0; activeNodeIndex < numberOfActiveNodes; ++activeNodeIndex )
	{
		// Access the index of the node in the grid from the active node index
		const int refNode                       = activeNodes[activeNodeIndex];

		// Get the previous temperature, porosity, VES and current chemical compaction
		const double currentPorosity            = currentPorosityGrid[refNode];

		// In case the porosity already reached the minimum porosity
		if( currentPorosity > MinimumPorosity ) {
		      
		const double chemicalCompaction         = chemicalCompactionGrid[refNode];
		const double currentTemperature         = currentTemperatureGrid[refNode];
		const double currentVES                 = currentVESGrid[refNode];

		//Access the lithology parameters
		const int lithoID                      = m_lithologyMap[refNode];
		const SchneiderLithology & litho        = m_lithologyList[lithoID];

		const double activationEnergy           = litho.m_activationEnergy;           //[J/mol]
		const double referenceSolidViscosity    = litho.m_referenceSolidViscosity;    //[GPa/my]

		// Compute the new chemical compaction at the end of the time step
		// Units to use are : [K], [Pa], [J] or [Ws], [my], [mol]
		const double temperatureTerm  = 1.0 / ( std::max( currentTemperature , RockViscosityReferenceTemperature  ) + CelciusToKelvin ) - 1.0 / ( RockViscosityReferenceTemperature + CelciusToKelvin );
		const double solidViscosity   = 1.0e9 * referenceSolidViscosity * exp( activationEnergy * temperatureTerm / GasConstant );
		const double result           = timeStep * ( 1.0 - currentPorosity ) * currentVES / solidViscosity;
      assert( ("The computation of the chemical compaction returns not number value", !std::isnan( result )) );

		//Store the result in the currentGrid
		
		chemicalCompactionGrid[ refNode ] =  std::max( chemicalCompaction - result, -1.0 );
		}//end if 
	} // end of the path through active nodes
}


void SchneiderCompactionCalculator :: setLithologies( const Grid & grid )
{
	const double * activationEnergy              = grid.getActivationEnergy   ();
	const double * referenceViscosity            = grid.getReferenceViscosity ();
	const int numberLithologies                  = grid.getNumberOfLithologies  ();
	m_lithologyList.resize( numberLithologies );

	for ( int lithoIndex = 0 ; lithoIndex < numberLithologies ; ++lithoIndex )
	{
		m_lithologyList[lithoIndex].m_activationEnergy         = activationEnergy[lithoIndex];
		m_lithologyList[lithoIndex].m_referenceSolidViscosity  = referenceViscosity [lithoIndex];
	}
	grid.getLithologyMap( m_lithologyMap );
}


};//end of namespace


