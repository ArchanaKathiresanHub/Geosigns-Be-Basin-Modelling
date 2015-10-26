#include "FlowLines.h"
#include "Property.h"
#include "Mesh.h"

#include <Interface/GridMap.h>

#include <Inventor/SbVec.h>
#include <Inventor/nodes/SoLineSet.h>
#include <MeshVizXLM/MbVec3.h>

SoLineSet* generateFlowLines(const FlowDirectionProperty& values, int startK, const SnapshotTopology& topology)
{
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

        while (true)
        {
          MbVec3d center = topology.getCellCenter(ii, jj, kk);
          vertices.emplace_back((float)center[0], (float)center[1], (float)center[2]);
          nverts++;

          MbVec3<int32_t> deltas = values.getDeltas(ii, jj, kk);
          if (deltas[0] == 0 && deltas[1] == 0 && deltas[2] == 0)
            break;

          ii += deltas[0];
          jj += deltas[1];
          kk -= deltas[2];

          if (
            ii < 0 || ii >= numI ||
            jj < 0 || jj >= numJ ||
            kk < 0 || kk >= numK ||
            topology.isDead(ii, jj, kk))
          {
            break;
          }
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
