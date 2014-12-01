#include "SurfaceGridMap.h"

namespace migration {

SurfaceGridMap::SurfaceGridMap(const GridMap* gridMap, unsigned int index):
   m_gridMap(gridMap),
   m_index(index),
   m_status(CREATED)
{
   if (m_gridMap && m_gridMap->retrieved())
      m_status = RETRIEVED;
}

SurfaceGridMap::~SurfaceGridMap()
{
   if (m_status == RETRIEVED)
      restoreData();
}


void SurfaceGridMap::retrieveData()
{
   //   assert(m_status == CREATED || m_status == RESTORED);
   if (m_gridMap)
      m_gridMap->retrieveData();
   m_status = RETRIEVED;
}

void SurfaceGridMap::restoreData()
{
   assert(m_status == RETRIEVED);
   if (m_gridMap)
      m_gridMap->restoreData();
   m_status = RESTORED;
}

void SurfaceGridMap::release()
{
   assert(m_status == CREATED || m_status == RESTORED);
   m_gridMap = 0;
}

double SurfaceGridMap::operator[](const Tuple2<unsigned int>& ij) const
{
   assert(m_status == RETRIEVED);
   assert(m_gridMap);
   assert(m_index < m_gridMap->getDepth());
   double result = m_gridMap->getValue(ij[0], ij[1], m_index);
   assert(result != m_gridMap->getUndefinedValue());
   return result;
}

} // namespace migration
