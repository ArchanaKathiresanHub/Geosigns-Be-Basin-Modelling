/**
 * \file ChemicalCompactionWalderhaugGrid.h
 * \brief ChemicalCompactionWalderhaugGrid class which link WalderhaugCompactionCalculator class to fem_grid objects
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 */

#ifndef FASTCAULDRON_CHEMICALCOMPACTIONWALDERHAUGGRID_H
#define FASTCAULDRON_CHEMICALCOMPACTIONWALDERHAUGGRID_H


#include "ChemicalCompactionGrid.h"
#include <layer.h>
#include <vector>


/*! \class ChemicalCompactionWalderhaugGrid
 * \brief Get and set data between WalderhaugCompactionCalculator class and main code in order to compute chemical compaction
 *
 * Class derived from ChemicalCompactionGrid class
 * Provide data in order to compute chemical compaction to WalderhaugCompactionCalculator class and set computed data in main code
 *
 */
class ChemicalCompactionWalderhaugGrid : public ChemicalCompactionGrid
{
public:

	/*!
	 * \brief Constructor of the class ChemicalCompactionWalderhaugGrid
	 * Class derived from ChemicalCompactionGrid
	 */
	ChemicalCompactionWalderhaugGrid( DM* mapViewOfDomain,
			const LayerList & layerList );

	/*!
	 * \brief Destructor of the class ChemicalCompactionWalderhaugGrid
	 */
	~ ChemicalCompactionWalderhaugGrid();

	/*!
	 * \brief Get the array of previous temperature
	 */
	virtual const double *getPreviousTemperature() const;

	/*!
	 * \brief Get the array of current temperature
	 */
	virtual const double * getCurrentTemperature() const;

	/*!
	 * \brief Get the array of porosity
	 */
	virtual const double * getPorosity() const;

	/*!
	 * \brief Create the calculator of chemical compaction with Walderhaug model
	 */
	virtual ChemicalCompactionCalculator* createChemicalCompaction();

	/*!
	 * \brief Empty the grid in order to load new parameters
	 */
	virtual void emptyDerivedGrid();

	/*!
	 * \brief Get the quartz grain size [cm]
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
	 */
	virtual const double * getQuartzGrainSize() const;
	/*!
	 * \brief Get the coating clay factor [fraction of surface]
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
	 */
	virtual const double * getCoatingClayFactor() const;
	/*!
	 * \brief Get the quartz fraction [fraction of volume]
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
	 */
	virtual const double * getQuartzFraction() const;


private:

	/*! \class Properties
	 * \brief Allocate and desallocate the grid properties
	 *
	 * Class allowing the allocation of the grid properties during the creation of the object
	 * and the desallocation of the same properties with the object destruction
	 */
	class Properties : public ChemicalCompactionGrid::Properties
	{
	public:

		/*!
		 * \brief Constructor of the class Properties
		 * Allocate the layer properties needed for the Walderhaug computation of chemical compaction
		 */
		Properties( const LayerProps & layer );

		/*!
		 * \brief Destructor of the class Properties
		 * Desallocate the layer properties needed for the Walderhaug computation of chemical compaction
		 */
		virtual ~Properties();

		/*!
		 * \brief Store the values of the porosity, the current and previous temperature and the chemical compaction
		 *  in order to compute chemical compaction
		 *
		 * \param i, j, k: int the coordinates of the node
		 * \param node: int the index of the node in the arrays
		 * \param grid: ChemicalCompactionGrid* the pointer to the grid where to store the properties
		 */
		virtual void storeProperties( int i, int j, int k, int node, ChemicalCompactionGrid* grid );


	private:
		Properties ( const Properties& properties );
		Properties * operator=( const Properties& properties );
		
		const LayerProps* m_layer;        /*!< current layer */
		double *** m_previousTemperature; /*!< array of the temperature of the previous snapshot */
		double *** m_currentTemperature;  /*!< array of the temperature of the current snapshot */
		double *** m_chemicalCompaction;  /*!< array of the chemical compaction */
		PETSC_3D_Array m_porosity;        /*!< array of the porosity */
	};

	/*!
	 * \brief Create an instance of Properties
	 * \param layer: LayerProps& the current layer
	 */
	virtual Properties * getProperties( const LayerProps& layer );

	/*!
	 * \brief Store the needed parameters of the lithology
	 * Here store the grain size, the coating clay factor and the fraction of quartz of each lithology
	 * \param currentLithology: CompoundLithology* the current lithology
	 */
	virtual void storeParameters( const CompoundLithology* lithology );

	std::vector < double > m_currentTemperature;  /*!< vector of the temperature at the current snapshot */
	std::vector < double > m_previousTemperature; /*!< vector of the temperature at the previous snapshot */
	std::vector < double > m_porosity;            /*!< vector of the porosity at the current snapshot */

	std::vector< double > m_grainSize;            /*!< vector of the size of the grain of quartz */
	std::vector< double > m_coatingClay;          /*!< vector of the coating clay factor */
	std::vector< double > m_fractionQuartz;       /*!< vector of the fraction of quartz */
};

#endif
