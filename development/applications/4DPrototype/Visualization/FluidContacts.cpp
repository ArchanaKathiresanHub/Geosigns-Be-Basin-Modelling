#include "FluidContacts.h"

#include <MeshVizXLM/MxTimeStamp.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>

#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoLineSet.h>

#include <stack>

struct GridCoord
{
  int i, j;
};

SoLineSet* buildIsoLines(
  const MiVolumeMeshCurvilinear& mesh,
  const std::vector<Project::Trap>& traps)
{
  std::vector<SbVec3f> vertices;

  auto const& geometry = mesh.getGeometry();
  auto const& topology = mesh.getTopology();

  size_t ni = topology.getNumCellsI();
  size_t nj = topology.getNumCellsJ();
  std::vector<bool> processed(ni * nj, false);
  std::stack<GridCoord> coords;

  MbVec3d p00 = geometry.getCoord(0, 0, 0);
  MbVec3d p01 = geometry.getCoord(0, 1, 0);
  MbVec3d p10 = geometry.getCoord(1, 0, 0);

  float dx = (float)(p10[0] - p00[0]);
  float dy = (float)(p01[1] - p00[0]);

  int di[] = { -1, 0, 1, 0 };
  int dj[] = { 0, -1, 0, 1 };

  for (auto const& trap : traps)
  {
    SbVec3f p = trap.leakagePoint;
    float z0 = p[2];
    //float z1 = (float)trap.gasOilContactDepth;
    float z2 = (float)trap.oilWaterContactDepth;

    GridCoord coord;
    coord.i = (int)floor(p[0] / dx + .5f);
    coord.j = (int)floor(p[1] / dy + .5f);

    if (!processed[coord.i + coord.j * ni])
    {
      coords.push(coord);
      processed[coord.i + coord.j * ni] = true;
    }

    while (!coords.empty())
    {
      coord = coords.top();
      coords.pop();

      MbVec3d q[4];
      q[0] = geometry.getCoord(coord.i, coord.j, 0);
      q[1] = geometry.getCoord(coord.i+1, coord.j, 0);
      q[2] = geometry.getCoord(coord.i+1, coord.j+1, 0);
      q[3] = geometry.getCoord(coord.i, coord.j+1, 0);

      bool higher = true;
      bool b[4];
      for (int i = 0; i < 4; ++i)
      {
        b[i] = q[i][2] >= z2; //true: in, false: out
        if (q[i][2] < z0)
          higher = false;
      }

      if (higher)
        continue;

      for (int i = 0, j = 3; i < 4; j = i++)
      {
        if (b[j] != b[i])
        {
          MbVec3d newP = q[j] + ((z2 - q[j][2]) / (q[i][2] - q[j][2])) * (q[i] - q[j]);
          vertices.push_back(
            SbVec3f(
            (float)newP[0],
            (float)newP[1],
            (float)newP[2]));
        }

        if (b[j] || b[i])
        {
          GridCoord newCoord = { coord.i + di[i], coord.j + dj[i] };
          if (
	    newCoord.i >= 0 && newCoord.i < (int)ni &&
	    newCoord.j >= 0 && newCoord.j < (int)nj &&
            !topology.isDead(newCoord.i, newCoord.j, 0) &&
            !processed[newCoord.i + newCoord.j * ni])
          {
            coords.push(newCoord);
            processed[newCoord.i + newCoord.j * ni] = true;
          }
        }
      }
    }
  }

  SoVertexProperty* vertexProperty = new SoVertexProperty;
  vertexProperty->vertex.setValues(0, (int)vertices.size(), &vertices[0]);

  SoLineSet* lineSet = new SoLineSet;
  lineSet->vertexProperty = vertexProperty;
  for (size_t i = 0; i < vertices.size() / 2; ++i)
    lineSet->numVertices.set1Value((int)i, 2);

  return lineSet;
}

