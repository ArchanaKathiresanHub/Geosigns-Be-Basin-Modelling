/*
 * \file ChemicalCompactionWalderhaugGrid.C
 */

#include "ChemicalCompactionWalderhaugGrid.h"
#include "../../../libraries/GeoPhysics/src/WalderhaugCompactionCalculator.h"


ChemicalCompactionWalderhaugGrid :: ChemicalCompactionWalderhaugGrid( DM* mapViewOfDomain,
		const LayerList & layerList ) :
		ChemicalCompactionGrid( mapViewOfDomain, layerList ),
		m_currentTemperature ( getNumberOfNodes( mapViewOfDomain, layerList) ),
		m_previousTemperature ( getNumberOfNodes( mapViewOfDomain, layerList) ),
		m_porosity ( getNumberOfNodes( mapViewOfDomain, layerList) )
{

}

ChemicalCompactionWalderhaugGrid :: ~ChemicalCompactionWalderhaugGrid()
{

}

const double *ChemicalCompactionWalderhaugGrid :: getPreviousTemperature() const
{
	return &m_previousTemperature[0];
}

const double *ChemicalCompactionWalderhaugGrid :: getCurrentTemperature() const
{
	return &m_currentTemperature[0];
}

const double *ChemicalCompactionWalderhaugGrid :: getPorosity() const
{
	return &m_porosity[0];
}

ChemicalCompactionCalculator* ChemicalCompactionWalderhaugGrid::createChemicalCompaction()
{
	return new WalderhaugCompactionCalculator ();
}

void ChemicalCompactionWalderhaugGrid::emptyDerivedGrid()
{
	m_chemicalCompaction.clear();
	m_currentTime = 0.0;
	m_previousTime = 0.0;
	m_validNodes.clear();
	m_numberValidNodes = 0;

	m_previousTemperature.clear();
	m_currentTemperature.clear();
	m_porosity.clear();
	m_grainSize.clear();            
	m_coatingClay.clear();
	m_fractionQuartz.clear();
}

/*
 * class Properties
 */

ChemicalCompactionWalderhaugGrid :: Properties :: Properties( const LayerProps & layer ):
  m_layer( &layer ),
  m_porosity( layer.layerDA, layer.Porosity )
{
	DMDAVecGetArray(layer.layerDA,
			layer.Current_Properties ( Basin_Modelling::Temperature ),
			&m_currentTemperature);
	DMDAVecGetArray(layer. layerDA,
			layer. Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
			&m_chemicalCompaction);
	DMDAVecGetArray(layer. layerDA,
			layer.Previous_Properties ( Basin_Modelling::Temperature ),
			&m_previousTemperature);
}

ChemicalCompactionWalderhaugGrid :: Properties :: ~Properties()
{
	DMDAVecRestoreArray(m_layer->layerDA,
			m_layer->Current_Properties ( Basin_Modelling::Temperature ),
			&m_currentTemperature);
	DMDAVecRestoreArray(m_layer->layerDA,
			m_layer->Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
			&m_chemicalCompaction);
	DMDAVecRestoreArray(m_layer->layerDA,
			m_layer->Previous_Properties ( Basin_Modelling::Temperature ),
			&m_previousTemperature);
}

void ChemicalCompactionWalderhaugGrid :: Properties :: storeProperties( int i, int j, int k, int node, ChemicalCompactionGrid* grid )
{
	ChemicalCompactionWalderhaugGrid* walderhaugGrid = dynamic_cast<ChemicalCompactionWalderhaugGrid*> (grid);
	walderhaugGrid->m_porosity[node]             = m_porosity(k, j, i);
	walderhaugGrid->m_previousTemperature[node]  = m_previousTemperature[k][j][i];
	walderhaugGrid->m_currentTemperature[node]   = m_currentTemperature[k][j][i];
	walderhaugGrid->m_chemicalCompaction[node]   = m_chemicalCompaction[k][j][i];

}

ChemicalCompactionWalderhaugGrid :: Properties* ChemicalCompactionWalderhaugGrid :: getProperties( const LayerProps& layer)
{
	return new ChemicalCompactionWalderhaugGrid :: Properties( layer );
}

void ChemicalCompactionWalderhaugGrid :: storeParameters( const CompoundLithology* lithology )
{
	m_grainSize.push_back      ( lithology -> quartzGrainSize()   );
	m_coatingClay.push_back    ( lithology -> coatingClayFactor() );
	m_fractionQuartz.push_back ( lithology -> quartzFraction()    );
}

const double * ChemicalCompactionWalderhaugGrid::getQuartzGrainSize() const
{
	return &m_grainSize[0];
}

const double * ChemicalCompactionWalderhaugGrid::getClayCoatingFactor() const
{
	return &m_coatingClay[0];
}

const double * ChemicalCompactionWalderhaugGrid::getQuartzFraction() const
{
	return &m_fractionQuartz[0];
}


