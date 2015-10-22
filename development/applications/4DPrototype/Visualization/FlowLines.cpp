#include "FlowLines.h"
#include "GridMapCollection.h"
#include "Mesh.h"

#include <Interface/GridMap.h>

#include <Inventor/SbVec.h>
#include <Inventor/nodes/SoLineSet.h>
#include <MeshVizXLM/MbVec3.h>

SoLineSet* generateFlowLines(const std::vector<const DataAccess::Interface::GridMap*>& values, int startK, const SnapshotTopology& topology)
{
  if (values.empty())
    return nullptr;

  std::vector<SbVec3f> vertices;
  std::vector<int32_t> numVertices;

  const int numI = (int)topology.getNumCellsI();
  const int numJ = (int)topology.getNumCellsJ();
  const int numK = (int)topology.getNumCellsK();

  for (size_t i = 0; i < numI; ++i)
  {
    for (size_t j = 0; j < numJ; ++j)
    {
      if (!topology.isDead(i, j, startK))
      {
        int ii = (int)i;
        int jj = (int)j;
        int kk = (int)startK;

        int nverts = 0;
        unsigned int localK = 1;

        auto iter = values.rbegin();
        while (iter != values.rend())
        {
          MbVec3d center = topology.getCellCenter(ii, jj, kk);
          vertices.emplace_back((float)center[0], (float)center[1], (float)center[2]);
          nverts++;

          int code = (int)(*iter)->getValue(ii, jj, localK);
          if (code == 0 || code == 99999)
            break;

          code += 111;
          int dk = code / 100 - 1;
          int dj = (code % 100) / 10 - 1;
          int di = (code % 10) - 1;

          if (
            di < -1 || di > 1 ||
            dj < -1 || dj > 1 ||
            dk <  0 || dk > 1)
          {
            break;
          }

          ii += di;
          jj += dj;
          kk -= dk;

          if (
            ii < 0 || ii >= numI ||
            jj < 0 || jj >= numJ ||
            kk < 0 || ii >= numK ||
            topology.isDead(ii, jj, kk))
          {
            break;
          }

          if (dk == 1 && --localK == 0 && ++iter != values.rend())
            localK = (*iter)->getDepth() - 1;
        }

        if (nverts > 0)
          numVertices.push_back((int)nverts);
      }
    }
  }

  SoVertexProperty* vertexProperty = new SoVertexProperty;
  vertexProperty->vertex.setValues(0, (int)vertices.size(), &vertices[0]);

  SoLineSet* lineSet = new SoLineSet;
  lineSet->vertexProperty = vertexProperty;
  lineSet->numVertices.setValues(0, (int)numVertices.size(), &numVertices[0]);

  return lineSet;
}
