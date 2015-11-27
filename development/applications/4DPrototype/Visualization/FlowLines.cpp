#include "FlowLines.h"
#include "Property.h"
#include "Mesh.h"

#include <Inventor/SbVec.h>
#include <Inventor/nodes/SoLineSet.h>
#include <MeshVizXLM/MbVec3.h>

MbVec3d getCellCenter(const MiGeometryIjk& geometry, size_t i, size_t j, size_t k)
{
  return .125 * (
    geometry.getCoord(i, j, k) +
    geometry.getCoord(i + 1, j, k) +
    geometry.getCoord(i, j + 1, k) +
    geometry.getCoord(i + 1, j + 1, k) +
    geometry.getCoord(i, j, k + 1) +
    geometry.getCoord(i + 1, j, k + 1) +
    geometry.getCoord(i, j + 1, k + 1) +
    geometry.getCoord(i + 1, j + 1, k + 1));
}

MbVec3<int32_t> decodeFlowDirection(int code)
{
  if (code == 0 || code == (int)DataAccess::Interface::DefaultUndefinedMapValue)
    return MbVec3<int32_t>();

  code += 111;
  int dk = code / 100 - 1;
  int dj = (code % 100) / 10 - 1;
  int di = (code % 10) - 1;

  if (
    dk <  0 || dk > 1 ||
    dj < -1 || dj > 1 ||
    di < -1 || di > 1)
  {
    return MbVec3<int32_t>();
  }

  return MbVec3<int32_t>(di, dj, dk);
}


SoLineSet* generateFlowLines(const MiDataSetIjk<double>& values, int startK, const MiVolumeMeshCurvilinear& mesh)
{
  const MiGeometryIjk& geometry = mesh.getGeometry();
  const MiTopologyIjk& topology = mesh.getTopology();

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
          MbVec3d center = getCellCenter(geometry, ii, jj, kk);
          vertices.emplace_back((float)center[0], (float)center[1], (float)center[2]);
          nverts++;

          int code = (int)values.get(ii, jj, kk);
          MbVec3<int32_t> deltas = decodeFlowDirection(code);
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

        if (nverts == 1)
          vertices.erase(--vertices.end());

        if (nverts > 1)
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
