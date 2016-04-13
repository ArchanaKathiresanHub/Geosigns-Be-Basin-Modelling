#include <Interface/ProjectHandle.h>
#include <Interface/PropertyValue.h>
#include "Interface/Property.h"
#include "Interface/Formation.h"
#include "Interface/Reservoir.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Interface.h"

#include <fstream>

namespace di = DataAccess::Interface;

void exportData(const di::ProjectHandle* handle)
{
  std::unique_ptr<di::SnapshotList> snapshotList(handle->getSnapshots());
  std::sort(snapshotList->begin(), snapshotList->end(),
    [](const di::Snapshot* s1, const di::Snapshot* s2) { return s1->getTime() > s2->getTime(); });

  const di::Grid* grid = handle->getLowResolutionOutputGrid();
  unsigned int ni = grid->numI();
  unsigned int nj = grid->numJ();
  std::vector<unsigned int> depths;

  float* output = new float[ni * nj];
  std::ofstream out("Depth.dat", std::ofstream::binary);

  const di::Property* depthProperty = handle->findProperty("Depth");

  for (auto snapshot : *snapshotList)
  {
    unsigned int snapshotDepth = 0;

    std::unique_ptr<di::PropertyValueList> valueList(depthProperty->getPropertyValues(di::FORMATION, snapshot, 0, 0, 0));
    for (auto value : *valueList)
    {
      if (value->getFormation()->kind() == di::BASEMENT_FORMATION)
        continue;

      const di::GridMap* gridMap = value->getGridMap();
      unsigned int nk = (value == valueList->back()) 
        ? gridMap->getDepth() 
        : gridMap->getDepth() - 1;

      snapshotDepth += nk;

      for (unsigned int k = 0; k < nk; ++k)
      {
        float* p = output;

        for (unsigned int j = 0; j < nj; ++j)
          for (unsigned int i = 0; i < ni; ++i)
            *p++ = (float)gridMap->getValue(i, j, k);

        out.write((const char*)output, ni * nj * sizeof(float));
      }
    }

    depths.push_back(snapshotDepth);
  }

  const char* propertyNames[] = 
  {
    "Porosity",
    "Temperature",
    "Pressure"
  };

  for (int i = 0; i < 1; ++i)
  {
    const di::Property* prop = handle->findProperty(propertyNames[i]);
    std::ofstream propOut(std::string(propertyNames[i]) + ".dat", std::ostream::binary);

    for (auto snapshot : *snapshotList)
    {
      std::unique_ptr<di::PropertyValueList> valueList(prop->getPropertyValues(di::FORMATION, snapshot, 0, 0, 0));
      for (auto value : *valueList)
      {
        if (value->getFormation()->kind() == di::BASEMENT_FORMATION)
          continue;

        const di::GridMap* gridMap = value->getGridMap();
        unsigned int nk = gridMap->getDepth();
  
        for (unsigned int k = 0; k < nk - 1; ++k)
        {
          float* p = output;

          for (unsigned int j = 0; j < nj - 1; ++j)
            for (unsigned int i = 0; i < ni - 1; ++i)
              *p++ = .125f * (float)(
                gridMap->getValue(i,     j,     k) +
                gridMap->getValue(i + 1, j,     k) +
                gridMap->getValue(i,     j + 1, k) +
                gridMap->getValue(i + 1, j + 1, k) +
                gridMap->getValue(i,     j,     k + 1) +
                gridMap->getValue(i + 1, j,     k + 1) +
                gridMap->getValue(i,     j + 1, k + 1) +
                gridMap->getValue(i + 1, j + 1, k + 1));

          propOut.write((const char*)output, (ni - 1) * (nj - 1) * sizeof(float));
        }
      }
    }
  }

  std::ofstream indexOut("index.dat", std::ostream::binary);
  indexOut.write((const char*)&ni, sizeof(ni));
  indexOut.write((const char*)&nj, sizeof(nj));

  unsigned int n = (unsigned int)depths.size();
  indexOut.write((const char*)&n, sizeof(n));

  for (auto d : depths)
    indexOut.write((const char*)&d, sizeof(d));
}