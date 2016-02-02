#include "FlowLines.h"
#include "Property.h"
#include "Mesh.h"
#include "Project.h"
#include "DataSet.h"

#include <Inventor/SbVec.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoGeometryShader.h>
#include <Inventor/nodes/SoFragmentShader.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <MeshVizXLM/MbVec3.h>

const char* vertexShaderSrc =
  "void main()\n"
  "{\n"
  "  gl_FrontColor = gl_Color;\n"
  "  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
  "}\n";

const char* geometryShaderSrc =
  "#version 150 core\n"
  "layout(lines) in;\n"
  "layout(line_strip, max_vertices=6) out;\n"
  "void main()\n"
  "{\n"
  "  gl_FrontColor = gl_in[0].gl_FrontColor;\n"
  "  vec4 p0 = gl_in[0].gl_Position;\n"
  "  vec4 p1 = gl_in[1].gl_Position;\n"
  "  vec4 vt = p1 - p0;\n"
  "  vec4 vp = vec4(-vt.y, vt.x, vt.z, vt.w);\n"
  "  gl_Position = p0;\n"
  "  EmitVertex();\n"
  "  vec4 p2 = p0 + 0.8 * vt;\n"
  "  gl_Position = p2;\n"
  "  EmitVertex();\n"
  "  gl_Position = p2 + 0.05 * vp;\n"
  "  EmitVertex();\n"
  "  gl_Position = p1;\n"
  "  EmitVertex();\n"
  "  gl_Position = p2 - 0.05 * vp;\n"
  "  EmitVertex();\n"
  "  gl_Position = p2;\n"
  "  EmitVertex();\n"
  "}\n";

const char* fragmentShaderSrc =
  "//!oiv_include <Inventor/oivDepthPeeling_frag.h>\n"
  "void main()\n"
  "{\n"
  "  OivDepthPeelingOutputColor(gl_Color);\n"
  "}\n";

SoShaderProgram* createFlowLinesVectorShader()
{
  auto vertexShader = new SoVertexShader;
  vertexShader->sourceProgram = vertexShaderSrc;
  vertexShader->sourceType = SoShaderObject::GLSL_PROGRAM;
  vertexShader->isActive = true;

  auto geometryShader = new SoGeometryShader;
  geometryShader->sourceProgram = geometryShaderSrc;
  geometryShader->sourceType = SoShaderObject::GLSL_PROGRAM;
  geometryShader->isActive = true;

  auto fragmentShader = new SoFragmentShader;
  fragmentShader->sourceProgram = fragmentShaderSrc;
  fragmentShader->sourceType = SoShaderObject::GLSL_PROGRAM;
  fragmentShader->isActive = true;

  auto program = new SoShaderProgram;
  program->shaderObject.set1Value(0, vertexShader);
  program->shaderObject.set1Value(1, geometryShader);
  program->shaderObject.set1Value(2, fragmentShader);
  program->geometryInputType = SoShaderProgram::LINES_INPUT;
  program->geometryOutputType = SoShaderProgram::LINE_STRIP_OUTPUT;
  program->maxGeometryOutputVertices = 6;

  return program;
}

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
    dk < -1 || dk > 1 ||
    dj < -1 || dj > 1 ||
    di < -1 || di > 1)
  {
    return MbVec3<int32_t>();
  }

  return MbVec3<int32_t>(di, dj, dk);
}

std::shared_ptr<MiDataSetIj<double> > generateLeakageProperty(const Project& project, size_t snapshotIndex, int reservoirId)
{
  const double undefined = 99999.0;
  const char* leakage = "ResRockLeakage";

  int propertyId = project.getPropertyId(leakage);

  auto prop = project.createReservoirProperty(snapshotIndex, reservoirId, propertyId);
  if (!prop)
    return nullptr;

  auto dim = project.getProjectInfo().dimensions;

  size_t di = (size_t)(dim.numCellsIHiRes / dim.numCellsI);
  size_t dj = (size_t)(dim.numCellsJHiRes / dim.numCellsJ);

  size_t ni = (size_t)(dim.numCellsI + 1);
  size_t nj = (size_t)(dim.numCellsJ + 1);
  double* values = new double[ni * nj];
  double* p = values;
  for (size_t j = 0; j < nj; ++j)
  {
    for (size_t i = 0; i < ni; ++i)
    {
      double sum = 0.0;
      int n = 0;

      for (size_t jj = 0; jj < dj; ++jj)
      {
        for (size_t ii = 0; ii < di; ++ii)
        {
          double v = prop->get(i * di + ii, j * dj + jj, 0u);
          if (v != undefined)
          {
            sum += v;
            n++;
          }
        }
      }

      *p++ = (n > 0) ? sum : undefined;
    }
  }

  return std::make_shared<DataSetIj>(values, ni, nj);
}

std::shared_ptr<MiDataSetIj<double> > generateExpulsionProperty(const Project& project, size_t snapshotIndex, int formationId)
{
  const double undefined = 99999.0;

  const char* oil = "OilExpelledCumulative";
  const char* gas = "HcGasExpelledCumulative";

  int oilId = project.getPropertyId(oil);
  int gasId = project.getPropertyId(gas);

  auto oilData = project.createFormation2DProperty(snapshotIndex, formationId, oilId);
  auto gasData = project.createFormation2DProperty(snapshotIndex, formationId, gasId);
  
  if (!oilData || !gasData)
    return nullptr;

  auto dim = project.getProjectInfo().dimensions;

  size_t ni = (size_t)(dim.numCellsI + 1);
  size_t nj = (size_t)(dim.numCellsJ + 1);
  double* values = new double[ni * nj];

  double* p = values;
  for (size_t j=0; j < nj; ++j)
  {
    for (size_t i=0; i < ni; ++i)
    {
      double v1 = oilData->get(i, j);
      double v2 = gasData->get(i, j);

      double total = (v1 != undefined && v2 != undefined)
        ? v1 + v2
        : undefined;

      *p++ = total;
    }
  }

  if (snapshotIndex < project.getSnapshotCount() - 1)
  {
    auto prevOilData = project.createFormation2DProperty(snapshotIndex + 1, formationId, oilId);
    auto prevGasData = project.createFormation2DProperty(snapshotIndex + 1, formationId, gasId);

    if (prevOilData && prevGasData)
    {
      double* p = values;
      for (size_t j = 0; j < nj; ++j)
      {
        for (size_t i = 0; i < ni; ++i)
        {
          if (*p != undefined)
          {
            double v1 = prevOilData->get(i, j);
            double v2 = prevGasData->get(i, j);

            double total = (v1 != undefined && v2 != undefined)
              ? v1 + v2
              : undefined;

            if (total != undefined)
              *p -= total;
            else
              *p = undefined;
          }

          ++p;
        }
      }
    }
  }

  return std::make_shared<DataSetIj>(values, ni, nj);
}

SoLineSet* generateFlowLines(
  const MiDataSetIjk<double>& values, 
  std::shared_ptr<MiDataSetIj<double> > expulsion, 
  const MiVolumeMeshCurvilinear& mesh,
  int startK,
  int step,
  double threshold)
{
  const MiGeometryIjk& geometry = mesh.getGeometry();
  const MiTopologyIjk& topology = mesh.getTopology();

  std::vector<SbVec3f> vertices;
  std::vector<int32_t> numVertices;

  const int numI = (int)topology.getNumCellsI();
  const int numJ = (int)topology.getNumCellsJ();
  const int numK = (int)topology.getNumCellsK();

  for (int i = 0; i < numI; i += step)
  {
    for (int j = 0; j < numJ; j += step)
    {
      double expulsionValue = expulsion ? expulsion->get(i, j) : 1.0;

      if (!topology.isDead(i, j, startK) && expulsionValue > threshold && expulsionValue != 99999.0)
      {
        int ii = i;
        int jj = j;
        int kk = startK;

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
