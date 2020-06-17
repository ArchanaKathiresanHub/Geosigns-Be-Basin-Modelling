#include "VoxetPropertyGrid.h"
#include "array.h"

using namespace ibs;

VoxetPropertyGrid::VoxetPropertyGrid ( const GridDescription& grid ) : m_grid ( grid ) {
   m_propertyValues = Array<float>::create3d ( m_grid.getVoxetNodeCount ( 2 ), m_grid.getVoxetNodeCount ( 1 ), m_grid.getVoxetNodeCount ( 0 ));
}

VoxetPropertyGrid::~VoxetPropertyGrid () {
   Array<float>::delete3d ( m_propertyValues );
}

float* VoxetPropertyGrid::getOneDData () const {
   return m_propertyValues [ 0 ][ 0 ];
}

const GridDescription& VoxetPropertyGrid::getGridDescription () const {
   return m_grid;
}
