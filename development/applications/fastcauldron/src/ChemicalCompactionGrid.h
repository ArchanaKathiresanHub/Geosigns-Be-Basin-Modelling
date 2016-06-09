/*
 * \file ChemicalCompactionGrid.h
 */

#ifndef FASTCAULDRON_CHEMICALCOMPACTIONGRID_H
#define FASTCAULDRON_CHEMICALCOMPACTIONGRID_H


#include <petsc.h>
#include <vector>
#include <string>
#include "layer.h"
#include "ChemicalCompactionCalculator.h"


/*! \class ChemicalCompactionGrid
 * \brief Abstract class allowing exchange between fem_grid data and the chemical compaction algorithms.
 *
 * For each algorithm corresponds a grid, which is a derived class of the ChemicalCompactionGrid abstract class.
 * For now, ChemicalCompactionSchneiderGrid and ChemicalCompactionWalderhaugGrid are the derived classes
 *
 */
class ChemicalCompactionGrid : public GeoPhysics :: ChemicalCompactionCalculator :: Grid
{
public:

	/*!
	 * \brief Constructor of the abstract class ChemicalCompactionGrid
	 */
	ChemicalCompactionGrid(
			DM* mapViewOfDomain,
			const LayerList & layerList
	);

	/*!
	 * \brief Destructor of the abstract class ChemicalCompactionGrid
	 */
	virtual ~ChemicalCompactionGrid();

	/*!
	 * \brief Compute number of grid nodes where chemical compaction is relevant to be processed.
	 * That means the sum of the number of nodes through the compactable layers.
	 * Typically if the chemical compaction is disable in the basin, this number should be 0.
	 * But in this case, the method wouldn't be called
	 */
	static int getNumberOfNodes( DM * mapViewOfDomain, const LayerList & layerList);

	/*!
	 * \brief Compute a locally unique node number on the basis of grid coordinates.
	 *  
	 * \param offset The offset for the current layer ie the sum of nodes for the compactable layers above the current one
	 * \param maxK   The number of local nodes in the Z = depth direction in the current layer 
	 * \param maxJ   The number of local nodes in the Y direction in the current layer
	 * \param k      The local grid coordinate in the Z = depth direction in the current layer
	 * \param j      The local grid coordinate in the Y-direction in the current layer
	 * \param i      The local grid coordinate in the X-direction in the current layer
	 * \return       A number that identifies a node in the 'chemical compaction grid'
	 */
	static int getNodeNumber( int offset, int maxK, int maxJ, int k, int j, int i )
	{
		return offset + k + maxK * ( j + maxJ * i );
	}

	/*!
	 * \brief Create a grid according to the name of the algorithm.
	 *
	 * Return an error if not a valid algorithm name.
	 * Are valid names: "Walderhaug", "Schneider" (July 2014)
	 *
	 * \param algorithmName: const std::string &  Name of the algorithm
	 */
	static ChemicalCompactionGrid * create( const std::string & algorithmName,
			DM* mapViewOfDomain,
			const LayerList & layerList );

	/*!
	 * \brief Get the time of the previous snapshot in million years [Ma].
	 */
	virtual double getPreviousTime() const { return m_previousTime; }

	/*!
	 * \brief Get the time of the current snapshot in million years [Ma].
	 */
	virtual double getCurrentTime() const { return m_currentTime; }

	/*!
	 * \brief Get the number of nodes of the grid.
	 */
	virtual int getSize() const { return m_size; }

	/*!
	 * \brief Get the array of actives nodes of the grid.
	 */
	virtual const int * getActiveNodes() const;

	/*!
	 * \brief Get the number of actives nodes of the grid.
	 */
	virtual int getNumberOfActiveNodes() const;

	/*!
	 * \brief Get the array of previous temperature.
	 * Overload in the Walderhaug Grid
	 */
	virtual const double * getPreviousTemperature() const;

	/*!
	 * \brief Get the array of current temperature.
	 */
	virtual const double * getCurrentTemperature() const = 0;

	/*!
	 * \brief Get the array of the current porosity.
	 */
	virtual const double * getPorosity() const = 0;

	/*!
	 * \brief Get the array of the current VES
	 * Overload in the Schneider grid
	 */
	virtual const double * getVES() const;

	/*!
	 * \brief Get the array of the previous chemical compaction.
	 */
	virtual double *setChemicalCompaction();

	/*!
	 * \brief Get the number of different lithologies.
	 * Used by Schneider chemical compaction.
	 */
	virtual int getNumberOfLithologies() const;

	/*!
	 * \brief Get the array of the reference lithology number of all nodes in the basin.
	 * The size is the one returned by getSize() ie the number of nodes.
	 *
	 * \param lithoMap: std::vector<int>& array of int corresponding to lithologies
	 */
	virtual void getLithologyMap( std::vector<int>& lithoMap ) const;

	/*!
	 * \brief Get the activation energy [kJ/mol].
	 * Used by Schneider chemical compaction.
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	virtual const double * getActivationEnergy() const { return 0; }

	/*!
	 * \brief Get the reference viscosity [Pa.s].
	 * Used by Schneider chemical compaction.
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	virtual const double * getReferenceViscosity() const { return 0; }

	/*!
	 * \brief Get the size of the grain of quartz [cm].
	 * Used by Walderhaug chemical compaction.
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	virtual const double * getQuartzGrainSize() const { return 0; }

	/*!
	 * \brief Get the fraction of quartz [fraction of total volume].
	 * Used by Walderhaug chemical compaction.
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	virtual const double * getQuartzFraction() const { return 0; }

	/*!
	 * \brief Get the coating clay factor [fraction of quartz surface area].
	 * Used by Walderhaug chemical compaction.
	 * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin.
	 */
	virtual const double * getClayCoatingFactor() const { return 0; }

	/*!
	 * \brief Go through the basin and get all the needed data.
	 * The differences between the algorithms are averload in each derived class through the class Properties.
	 *
	 * \param layerList: LayerList& the list of Layers in the basin
	 * \param isValisNeedle: Boolean2DArray& the array giving if a node is active or not
	 * \param areLithologiesChanged: bool By default is false, if the lithology map change, is true
	 */
	bool addLayers( DM* mapViewOfDomain,
			const LayerList & layerList,
			const Boolean2DArray & isValidNeedle,
			double previousTime,
			double currentTime
	);

	/*!
	 * \brief Go through the basin and set the computed chemical compaction in the Chemical Compaction property of the basin layers.
	 *
	 * \param layerList: LayerList& the list of Layers in the basin
	 * \param isValisNeedle: Boolean2DArray& the array giving if a node is active or not
	 */
	void exportToModel( const LayerList & layerList,
			const Boolean2DArray & isValidNeedle
	);

	/*!
	 * \brief Create a chemical compaction calculator object.
	 * Call the chemical compaction algorithm corresponding to the grid
	 */
	virtual ChemicalCompactionCalculator* createChemicalCompaction() = 0;

	/*!
	 * \brief Empty the grid in order to load new parameters
	 */
	void emptyGrid();

private:
	// To define here, before m_chemicalCompaction in order to avoid uninitialized values 
        int m_size;     /*!< number of nodes in the grid */	


protected:

	/*! \class Properties
	 * \brief Abstract class - Allocate and desallocate the grid properties.
	 *
	 * Class allowing the allocation of the grid properties during the creation of the object
	 * and the desallocation of the same properties with the object destruction
	 */
	class Properties
	{
	public:
		/*!
		 * \brief Destructor of the class Properties.
		 * Desallocate the grid properties.
		 */
		virtual ~Properties()
		{
		   //Empty here but defined in the derived classes
		};

		/*!
		 * \brief Store the current values of the properties in order to compute chemical compaction
		 *
		 * \param i, j, k: int the coordinates of the node
		 * \param node: int the index of the node in the arrays
		 * \param grid: ChemicalCompactionGrid* the pointer to the grid where to store the properties
		 */
		virtual void storeProperties( int i, int j, int k, int node, ChemicalCompactionGrid* grid ) = 0;
	};

	/*!
	 * \brief Create an instance of Properties.
	 * Properties is overload for Schneider and Walderhaug models.
	 * Different properties are loaded according to what is needed.
	 *
	 * \param layer: LayerProps& the current layer
	 */
	virtual Properties * getProperties( const LayerProps& layer ) = 0;

	/*!
	 * \brief Store the needed parameters of the lithology
	 *
	 * \param currentLithology: CompoundLithology* the current lithology
	 */
	virtual void storeParameters( const CompoundLithology*  currentLithology ) = 0;

	/*!
	 * \brief In case the grid as been emptied
	 */
	void initializeGrid();

	/*!
	 * \brief Empty the derived grid in order to load new parameters
	 */
	virtual void emptyDerivedGrid() = 0;

	///protected instead of private to be accessible from derived classes of Properties class
	std::vector < double > m_chemicalCompaction;            /*!< vector of double to store the value of chemical compaction */
	double m_currentTime;                                   /*!< time of the current snapshot in ma (million years) */
	double m_previousTime;                                  /*!< time of the previous snapshot in ma (million years) */
	std::vector < int > m_validNodes;                       /*!< vector of node index of valid nodes */
	int m_numberValidNodes;                                 /*!< number of valid nodes in the grid */

private:
	/*!
	 *  \brief Update the m_lithoMap vector if needed
	 */
	void changeLithoMap( const LayerProps& layer,
			int i,
			int j,
			int zs,
			int zm,
			int gridOffset,
			int ym,
			int xm,
			int ys,
			int xs );

	int m_numberLithologies;                                /*!< number of different lithologies in the grid */
	std::vector< const CompoundLithology * > m_listOfLitho; /*!< vector of pointers of lithologies in the basin */
	std::vector < int > m_lithoMap;                          /*!< vector of lithology index of nodes */
};

#endif
