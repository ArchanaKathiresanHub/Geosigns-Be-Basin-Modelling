//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/**
 * \file ChemicalCompactionSchneiderGrid.h
 * \brief ChemicalCompactionSchneiderGrid class which links SchneiderCompactionCalculator class to fem_grid objects.
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 */

#ifndef FASTCAULDRON_CHEMICALCOMPACTIONSCHNEIDERGRID_H
#define FASTCAULDRON_CHEMICALCOMPACTIONSCHNEIDERGRID_H

#include "ChemicalCompactionGrid.h"
#include <layer.h>
#include <vector>


/*! \class ChemicalCompactionSchneiderGrid
 * \brief Get and set data between SchneiderCompactionCalculator class and fem_grid objects in order to compute chemical compaction.
 *
 * Class derived from ChemicalCompactionGrid class.
 * Provide data in order to compute chemical compaction to SchneiderCompactionCalculator class and set computed data in main code
 *
 */
class ChemicalCompactionSchneiderGrid : public ChemicalCompactionGrid
{
public:

	/*!
	 * \brief Constructor
	 * 
	 * \param mapViewOfDomain: DM* the maps of the basin
	 * \param layerList: cont LayerList& the list of the layers in the basin
	 */
	ChemicalCompactionSchneiderGrid( DM* mapViewOfDomain,
         const bool isLegacy,
			const LayerList & layerList );

	/*!
	 * \brief Destructor of the ChemicalCompactionSchneiderGrid class
	 */
	~ ChemicalCompactionSchneiderGrid();

	/*!
	 * \brief Create a chemical compaction calculator (SchneiderCompactionCalculator) object.
	 * Call the chemical compaction algorithm corresponding to the grid.
	 */
	virtual ChemicalCompactionCalculator* createChemicalCompaction();

	/*!
	 * \brief Get the array of current temperature.
	 */
	virtual const double *getCurrentTemperature() const;

	/*!
	 * \brief Get the array of current porosity.
	 */
	virtual const double *getPorosity() const ;

	/*!
	 * \brief Get the array of current vertical effective stress.
	 */
	virtual const double *getVES() const;

	/*!
	 * \brief Empty the grid in order to load new parameters.
	 */
	virtual void emptyDerivedGrid();

	/*!
	 * \brief Get the activation energy [kJ/mol].
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	const double * getActivationEnergy() const;

	/*!
	 * \brief Get the reference viscosity [Pa.s].
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	const double * getReferenceViscosity() const;

   /*!
   * \brief is the behaviour legacy or new rock property library feature?
   */
   virtual const bool isLegacy() const;

private:

	/*! \class ChemicalCompactionSchneiderGrid::Properties
	 * \brief Allocate and desallocate the grid properties.
	 *
	 * Class allowing the allocation of the grid properties during the creation of the object
	 * and the desallocation of the same properties with the object destruction.
	 */
	class Properties : public ChemicalCompactionGrid::Properties
	{
	public:

		/*!
		 * \brief Constructor of the class Properties.
		 * Allocate the grid properties.
		 */
		explicit Properties( const LayerProps & layer );

		/*!
		 * \brief Destructor of the class Properties.
		 * Desallocate the grid properties.
		 */
		virtual ~Properties();

		/*!
		 * \brief  Storage of the properties.
		 * Copy the grid properties for one node.
		 * 
		 * \param i, j, k: coordinates of the node in the regular grid
		 * \param node: index of the node in the created grid
		 * \param grid: ChemicalCompactionGrid* the grid in which is the node 
		 */
		virtual void storeProperties( int i, int j, int k, int node, ChemicalCompactionGrid* grid );

	private:
		Properties (const Properties & properties );
		Properties & operator=( const Properties & properties );
		
		const LayerProps* m_layer;        /*!< the layer in which are the properties > */
		const double *** m_temperature;   /*!< the temperature array pointer > */
		double *** m_chemicalCompaction;  /*!< the chemical compaction array pointer > */
		const double *** m_ves;           /*!< the vertical effective stress array pointer > */
		const PETSC_3D_Array m_porosity; /*!< the porosity array pointer > */
	};

	/*!
	 * \brief Create an instance of the Properties class
	 * \param layer: LayerProps the layer in which are the properties
	 */
	virtual Properties * getProperties(const LayerProps& layer);

	/*!
	 * \brief Store the lithology parameters needed for the Schneider chemical compaction computation.
	 * \param lithology: CompoundLithology the current lithology
	 */
	virtual void storeParameters( const CompoundLithology* lithology );

	std::vector < double > m_currentTemperature;  /*!< vector of double to store the value of current temperature on every nodes >*/
	std::vector < double > m_porosity;            /*!< vector of double to store the value of porosity on every nodes >*/
	std::vector < double > m_ves;                 /*!< vector of double to store the value of vertical effective stress on every nodes >*/

	std::vector < double > m_activationEnergy;    /*!< vector of activation energy for every lithology of the basin >*/
	std::vector < double > m_referenceViscosity;  /*!< vector of reference viscosity for every lithology of the basin >*/

   const bool m_isLegacy; /*!< Legacy behaviour for minimum porosity?
                           * Flag for new rock property library (and new migration engine)
                           * 0 is the revised minimum porosity behaviour and additional mixing models
                           * 1 is simple minimum porosity behaviour and 2 mixing models*/
};

#endif
