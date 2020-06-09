//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/*
 * \file ChemicalCompactionSchneiderGrid.cpp
 */

#include "ChemicalCompactionSchneiderGrid.h"
#include "../../../libraries/GeoPhysics/src/SchneiderCompactionCalculator.h"

ChemicalCompactionSchneiderGrid :: ChemicalCompactionSchneiderGrid( DM* mapViewOfDomain,
      const bool isLegacy,
      const LayerList & layerList ) :
		ChemicalCompactionGrid( mapViewOfDomain, layerList ),
		m_currentTemperature ( getNumberOfNodes( mapViewOfDomain, layerList) ),
		m_porosity ( getNumberOfNodes( mapViewOfDomain, layerList) ),
		m_ves ( getNumberOfNodes( mapViewOfDomain, layerList) ),
      m_isLegacy(isLegacy)
{

}

ChemicalCompactionSchneiderGrid :: ~ChemicalCompactionSchneiderGrid()
{
	//Empty desctructor
}

ChemicalCompactionCalculator* ChemicalCompactionSchneiderGrid::createChemicalCompaction()
{
	return new SchneiderCompactionCalculator ();
}

const double *ChemicalCompactionSchneiderGrid :: getCurrentTemperature() const
{
	return &m_currentTemperature[0];
}

const double *ChemicalCompactionSchneiderGrid :: getPorosity() const
{
	return &m_porosity[0];
}

const double *ChemicalCompactionSchneiderGrid :: getVES() const
{
	return &m_ves[0];
}

void ChemicalCompactionSchneiderGrid::emptyDerivedGrid()
{
	m_chemicalCompaction.clear();
	m_currentTime = 0.0;
	m_previousTime = 0.0;
	m_validNodes.clear();
	m_numberValidNodes = 0;
	
	m_currentTemperature.clear();
	m_porosity.clear();
	m_ves.clear();
	m_activationEnergy.clear();
	m_referenceViscosity.clear();
}

/*
 * class Properties
 */

ChemicalCompactionSchneiderGrid :: Properties::Properties( const LayerProps & layer ):
  m_layer( &layer ),
  m_porosity( layer.layerDA, layer.Porosity )
{
	DMDAVecGetArray(layer.layerDA,
			layer.Current_Properties ( Basin_Modelling::Temperature ),
			&m_temperature);
	DMDAVecGetArray(layer. layerDA,
			layer. Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
			&m_chemicalCompaction);
	DMDAVecGetArray(layer. layerDA,
			layer. Current_Properties ( Basin_Modelling::VES_FP ),
			&m_ves);
}


ChemicalCompactionSchneiderGrid :: Properties::~Properties()
{
	DMDAVecRestoreArray( m_layer->layerDA,
			m_layer->Current_Properties ( Basin_Modelling::Temperature ),
			&m_temperature );
	DMDAVecRestoreArray( m_layer->layerDA,
			m_layer->Previous_Properties ( Basin_Modelling::Chemical_Compaction ),
			&m_chemicalCompaction );
	DMDAVecRestoreArray( m_layer->layerDA,
			m_layer->Current_Properties ( Basin_Modelling::VES_FP ),
			&m_ves );
}

void ChemicalCompactionSchneiderGrid :: Properties :: storeProperties( int i, int j, int k, int node, ChemicalCompactionGrid* grid )
{
	ChemicalCompactionSchneiderGrid* schneiderGrid = dynamic_cast<ChemicalCompactionSchneiderGrid*>(grid);

	schneiderGrid->m_porosity.resize(grid->getSize());
	schneiderGrid->m_currentTemperature.resize(grid->getSize());
	schneiderGrid->m_ves.resize(grid->getSize());

	schneiderGrid->m_porosity[node]            = m_porosity( k, j, i );
	schneiderGrid->m_currentTemperature[node]  = m_temperature[k][j][i];
	if( m_layer->isChemicalCompactionVesValueIsDefined() )
	{
		double chemicalCompactionValue = m_layer -> getChemicalCompactionVesValue();
		schneiderGrid->m_ves[node]= chemicalCompactionValue;
	}
	else
	{
		schneiderGrid->m_ves[node]  = m_ves[k][j][i];
	}
	schneiderGrid->m_chemicalCompaction[node]  = m_chemicalCompaction[k][j][i];
}

ChemicalCompactionSchneiderGrid :: Properties * ChemicalCompactionSchneiderGrid :: getProperties( const LayerProps& layer )
{
	return new ChemicalCompactionSchneiderGrid :: Properties( layer );
}

void ChemicalCompactionSchneiderGrid :: storeParameters( const CompoundLithology* lithology )
{
	m_activationEnergy.push_back   ( lithology -> activationEnergy()   );
	m_referenceViscosity.push_back ( lithology -> referenceViscosity() );
}


const double * ChemicalCompactionSchneiderGrid::getActivationEnergy() const
{
	return &m_activationEnergy[0];
}

const double * ChemicalCompactionSchneiderGrid::getReferenceViscosity() const
{
	return &m_referenceViscosity[0];
}

bool ChemicalCompactionSchneiderGrid::isLegacy() const
{
   return m_isLegacy; 
}


