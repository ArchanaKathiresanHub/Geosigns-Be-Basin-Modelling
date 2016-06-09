/*
 * \file ChemicalCompactionGrid.C
 */

#include "ChemicalCompactionGrid.h"
#include "ChemicalCompactionWalderhaugGrid.h"
#include "ChemicalCompactionSchneiderGrid.h"


#include "layer_iterators.h"
#include "LithologyManager.h"
#include <petscdmda.h>
#include <cassert>

using namespace std;

ChemicalCompactionGrid :: ChemicalCompactionGrid(
      DM* mapViewOfDomain,
		const LayerList & layerList ):
				m_size( getNumberOfNodes( mapViewOfDomain, layerList ) ),
				m_chemicalCompaction( m_size ),
				m_currentTime( 0.0 ),
				m_previousTime( 0.0 ),
				m_validNodes( 0 ),
				m_numberValidNodes( 0 ),
				m_numberLithologies( 0 ),
				m_listOfLitho( 0 ),
				m_lithoMap( m_size, -1 )
{

}

ChemicalCompactionGrid :: ~ChemicalCompactionGrid()
{
   //Empty destructor
}

int ChemicalCompactionGrid :: getNumberOfNodes( DM * mapViewOfDomain, const LayerList & layerList )
{
	//Return the sum of number of nodes on compactable layers
	int xm;
   int ym;
	DMDAGetCorners( *mapViewOfDomain, PETSC_NULL, PETSC_NULL, PETSC_NULL ,&xm, &ym, PETSC_NULL);

	Basin_Modelling::Layer_Iterator layers;
	layers.Initialise_Iterator ( layerList,
			Basin_Modelling::Descending,
			Basin_Modelling::Sediments_Only,
			Basin_Modelling::Active_And_Inactive_Layers
	);

	int nodeCount = 0;
	while ( ! layers.Iteration_Is_Done () )
	{
		LayerProps* layer = layers.Current_Layer();
		//if compactable layer
		if( layer -> Get_Chemical_Compaction_Mode() )
		{
			nodeCount += xm * ym  * ( layer -> getMaximumNumberOfElements() + 1 );
		}
		layers++;
	}

	return nodeCount;
}

ChemicalCompactionGrid * ChemicalCompactionGrid :: create( const std::string & algorithmName,
		DM* mapViewOfDomain,
		const LayerList & layerList)
{
	if ( algorithmName == "Schneider" )
	{
		return new ChemicalCompactionSchneiderGrid ( mapViewOfDomain,
				layerList );
	}
	else if ( algorithmName == "Walderhaug" )
	{
		return new ChemicalCompactionWalderhaugGrid ( mapViewOfDomain,
				layerList );
	}
	else
	{
		std::cerr << "Error message: Invalid Chemical compaction algorithm name" << std::endl;
		return 0;
	}
}

const int * ChemicalCompactionGrid :: getActiveNodes() const
{
	return &m_validNodes[0];
}

int ChemicalCompactionGrid :: getNumberOfActiveNodes() const
{
	return m_validNodes.size();
}

const double *ChemicalCompactionGrid :: getPreviousTemperature() const
{
	//Overload in Walderhaug model and not used for Schneider
	return 0;
}

const double *ChemicalCompactionGrid :: getVES() const
{
	//Overload in Schneider model and not used for Walderhaug
	return 0;
}

double *ChemicalCompactionGrid :: setChemicalCompaction()
{
	return &m_chemicalCompaction[0];
}

int ChemicalCompactionGrid :: getNumberOfLithologies() const
{
	return m_listOfLitho.size();
}

void ChemicalCompactionGrid :: getLithologyMap( std::vector<int>& lithoMap ) const
{
	lithoMap.clear();
	lithoMap.assign( m_lithoMap.begin(), m_lithoMap.end() );
}


bool ChemicalCompactionGrid :: addLayers( 
		DM* mapViewOfDomain,
		const LayerList & layerList,
		const Boolean2DArray & isValidNeedle,
		double previousTime,
		double currentTime)
{
	bool runChemicalCompaction = false;
	if( m_size == 0 )
	{
		m_size = getNumberOfNodes( mapViewOfDomain, layerList );
		initializeGrid();
	}
		
	// Traverse the layers from the basement to surface. This way values in the
	// properties array will stay at the same spot when new layers are added.
	m_previousTime          = previousTime;
	m_currentTime           = currentTime;

	m_validNodes.clear();

	Basin_Modelling::Layer_Iterator layers;
	layers.Initialise_Iterator( layerList,
			Basin_Modelling::Ascending,
			Basin_Modelling::Sediments_Only,
			Basin_Modelling::Active_Layers_Only
	);

	int gridOffset = 0;
	while (!layers.Iteration_Is_Done())
	{
		const LayerProps & layer = *layers.Current_Layer();

		//If not a compactable layer, do not load the data, go to next layer
		if (((LayerProps*)&layer)->Get_Chemical_Compaction_Mode())
		{
			// Get the dimensions
			int xs;
			int ys;
			int zs;
			int xm;
			int ym;
			int zm;

			DMDAGetCorners(layer.layerDA, &xs, &ys, &zs, &xm, &ym, &zm);

			const int layerGridSize = xm*ym*zm;

			// Get pointers to properties
			std::unique_ptr<Properties> properties(getProperties(layer));

			double***depth;
			DMDAVecGetArray(layer.layerDA,
				layer.Current_Properties(Basin_Modelling::Depth),
				&depth);

			const int activeNodeOffset = m_validNodes.size();
			m_validNodes.resize(activeNodeOffset + layerGridSize);

			int activeNode = activeNodeOffset;
			assert(("There cannot be more valid nodes than nodes in the grid", m_validNodes.size() <= m_size));

			// Traverse the grid in the layer
			for (int i = xs; i < xs + xm; i++)
			{
				for (int j = ys; j < ys + ym; j++)
				{
					// if node is inactive, it should be ignored
					if (isValidNeedle(i, j))
					{
						bool runChemicalCompaction = true;
						changeLithoMap(layer, i, j, zs, zm, gridOffset, ym, xm, ys, xs);

						for (int k = zs; k < zs + zm; k++)
						{
							int node = getNodeNumber(gridOffset, zm, ym, k - zs, j - ys, i - xs);

							assert(("Out of the m_validNodes array", activeNode < m_validNodes.size()));
							assert(("Try to access a negative index of the m_validNodes array", activeNode >= 0));
							assert(("Out of the m_chemicalCompaction array", node < m_size));
							assert(("Try to access a negative index of the m_chemicalCompaction array", node >= 0));

							double seaBottomDepth = FastcauldronSimulator::getInstance().getSeaBottomDepth(i, j, m_currentTime);

							// If, during deposition, the node is at the surface then do not start the chemical compaction calculation.
							if ((m_currentTime >= layer.depoage ||
								k > zs && layer.getSolidThickness(i, j, k - 1, m_currentTime) <= 0.0
								) && NumericFunctions::isEqual(depth[k][j][i], seaBottomDepth, 1.0e-06)
								)
							{
								/* then the node is inactive */
							}
							else
							{
								m_validNodes[activeNode] = node;
								activeNode++;
								//Store the properties values in the grid
								properties->storeProperties(i, j, k, node, this);
							}
						} //end if valid node
					}  // for k
				} // for j
			} // for i

			if (activeNode > 0)
			{
				m_validNodes.resize(activeNode);
			}

			assert(("Size of the lithoMap and nodes should be the same ", m_size == m_lithoMap.size()));
			DMDAVecRestoreArray(layer.layerDA,
				layer.Current_Properties(Basin_Modelling::Depth),
				&depth);
			gridOffset += layerGridSize;
		}

		// Go to next layer
		layers++;
	}
	return runChemicalCompaction;
}

void ChemicalCompactionGrid :: changeLithoMap( const LayerProps& layer, int i, int j, int zs, int zm, int gridOffset, int ym, int xm, int ys, int xs)
{
	const CompoundLithology*  currentLithology = layer.getLithology ( i, j );
	//Look if the current litho is stored. if not, store it
	int lithoID =
			std::distance(
					m_listOfLitho.begin(),
					std::find( m_listOfLitho.begin(), m_listOfLitho.end(), currentLithology)
			);

	if( lithoID == m_listOfLitho.size() )
	{
		m_listOfLitho.push_back( currentLithology );
		++m_numberLithologies;
		//Store the lithology parameters needed for the chemical compaction computation
		storeParameters( currentLithology );
	}
	//Store more than needed: some nodes are maybe upper than seafloor but won't be used
	for (int k = zs; k < zs+zm; ++k)
	{
		int node = getNodeNumber( gridOffset, zm, ym, k-zs, j-ys, i-xs);
		m_lithoMap[node] = lithoID;
	}
}


void ChemicalCompactionGrid :: exportToModel( const LayerList & layerList, const Boolean2DArray & isValidNeedle )
{

	// Traverse the layers from the basement to surface. This way values in the
	// properties array will stay at the same spot when new layers are added.
	Basin_Modelling::Layer_Iterator layers;
	layers.Initialise_Iterator ( layerList,
			Basin_Modelling::Ascending,
			Basin_Modelling::Sediments_Only,
			Basin_Modelling::Active_Layers_Only
	);


	// Maintain an offset in the ChemicalCompactionSchneiderGrid to remember where the layer starts
	int gridOffset = 0;

	// Iterate over all the layers
	while( ! layers.Iteration_Is_Done () )
	{

		const LayerProps& currentLayer = *layers.Current_Layer ();

		//If not a compactable layer, do not restore the data
		if (currentLayer.Get_Chemical_Compaction_Mode())
		{
			double***depth;
			DMDAVecGetArray(currentLayer.layerDA,
				currentLayer.Current_Properties(Basin_Modelling::Depth),
				&depth);


			// Get the dimensions of the current layer
			int xs, ys, zs, xm, ym, zm;
			DMDAGetCorners(currentLayer.layerDA, &xs, &ys, &zs, &xm, &ym, &zm);

			// determine its size
			const int layerGridSize = xm * ym * zm;

			// Create a property vector for chemical compaction. This vector doesn't need to be a
			// global vector since it doesn't need to know about ghost points
			double *** chemicalCompaction;
			DMDAVecGetArray(currentLayer.layerDA,
				currentLayer.Current_Properties(Basin_Modelling::Chemical_Compaction),
				&chemicalCompaction);

			for (int i = xs; i < xs + xm; i++)
			{
				for (int j = ys; j < ys + ym; j++)
				{
					if (isValidNeedle(i, j))
					{

						for (int k = zs; k < zs + zm; k++)
						{
							int node = getNodeNumber(gridOffset, zm, ym, k - zs, j - ys, i - xs);

							assert(("Out of the m_chemicalCompaction array", node < m_size));
							assert(("Try to access a negative index of the m_chemicalCompaction array", node >= 0));

							double seaBottomDepth = FastcauldronSimulator::getInstance().getSeaBottomDepth(i, j, m_currentTime);

							if ((m_currentTime >= currentLayer.depoage ||
								k > zs && currentLayer.getSolidThickness(i, j, k - 1, m_currentTime) <= 0.0
								) && NumericFunctions::isEqual(depth[k][j][i], seaBottomDepth, 1.0e-06)
								)
							{
								/* then the node is inactive */
							}
							else
							{
								chemicalCompaction[k][j][i] = m_chemicalCompaction[node];
							}

						}
					}
				}
			}

			// Clear any old reference to the previous layer.
			DMDAVecRestoreArray(currentLayer.layerDA,
				currentLayer.Current_Properties(Basin_Modelling::Chemical_Compaction),
				&chemicalCompaction);
			// Go the next layer
			gridOffset += layerGridSize;
		}
		layers++;
	}

}


void ChemicalCompactionGrid::emptyGrid()
{
	m_size = 0;
	m_numberLithologies = 0;
	m_listOfLitho.clear();
	m_lithoMap.clear();
	emptyDerivedGrid();
	
}

void ChemicalCompactionGrid::initializeGrid()
{
	m_chemicalCompaction.resize( m_size );
	m_lithoMap.resize( m_size, -1 );
}
