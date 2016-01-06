#include "FluidContacts.h"

#include <MeshVizXLM/MxTimeStamp.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>
#include <MeshVizXLM/mapping/nodes/MoLevelColorMapping.h>

#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/SbVec.h>

#include <stack>
#include <vector>

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

class DataSet : public MiDataSetIjk<double>
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double* m_values;

  size_t m_timestamp;

public:

  DataSet(double* values, size_t ni, size_t nj, size_t nk)
    : m_values(values)
    , m_numI(ni)
    , m_numJ(nj)
    , m_numK(nk)
    , m_timestamp(MxTimeStamp::getTimeStamp())
  {
  }

  DataSet(const DataSet&) = delete;
  DataSet& operator=(const DataSet&) = delete;

  virtual ~DataSet()
  {
    delete[] m_values;
  }

  virtual double get(size_t i, size_t j, size_t k) const
  {
    return m_values[k * m_numI * m_numJ + j * m_numI + i];
  }

  virtual MiMeshIjk::StorageLayout getLayout() const
  {
    return MiMeshIjk::LAYOUT_KJI;
  }

  virtual DataBinding getBinding() const
  {
    return MiDataSet::PER_CELL;
  }

  virtual size_t getTimeStamp() const
  {
    return m_timestamp;
  }

  virtual std::string getName() const { return "DataSet"; }
  virtual double getMin() const { return 0.0; }
  virtual double getMax() const { return 3.0; }
};

std::shared_ptr<MiDataSetIjk<double> > createFluidContactsProperty(
  const std::vector<Project::Trap>& traps, 
  const MiDataSetIjk<double>& trapIdProperty, 
  const MiVolumeMeshCurvilinear& mesh)
{
  auto const& topology = mesh.getTopology();
  auto const& geometry = mesh.getGeometry();

  size_t ni = topology.getNumCellsI();
  size_t nj = topology.getNumCellsJ();

  double* values = new double[ni * nj];
  memset(values, 0, ni * nj * sizeof(double));

  double* p = values;
  for (size_t j = 0; j < nj; ++j)
  {
    for (size_t i = 0; i < ni; ++i)
    {
      double val = 0.0;

      int id = (int)trapIdProperty.get(i, j, 0);
      if (id > 0 && id < 99999)
      {
        unsigned int index = id - 1;
        if (index < traps.size())
        {
          const Project::Trap& trap = traps[index];
          if (trap.id == id)
          {
            double depth = -geometry.getCoord(i, j, 0)[2];
            if (depth < trap.gasOilContactDepth)
              val = 3.0; // gas
            else if (depth < trap.oilWaterContactDepth)
              val = 2.0; // oil
            else
              val = 1.0; // water
          }
        }
      }

      *p++ = val;
    }
  }

  return std::make_shared<DataSet>(values, ni, nj, 1);
}

MoLevelColorMapping* createFluidContactsColorMap()
{
  const SbVec4f defaultColor(.5f, .5f, .5f, 1.f);
  const SbVec4f waterColor(0.f, 0.f, 1.f, 1.f); // blue
  const SbVec4f oilColor(1.f, 0.f, 0.f, 1.f);   // red
  const SbVec4f gasColor(0.f, 1.f, 0.f, 1.f);   // green

  float values[] = { -.5f, .5f, 1.5f, 2.5f, 3.5f };
  SbVec4f colors[] = { defaultColor, waterColor, oilColor, gasColor };

  auto map = new MoLevelColorMapping();
  map->values.setValues(0, 5, values);
  map->colors.setValues(0, 4, colors);

  return map;
}