#include "Seismic.h"
#include "GeometryUtil.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoCallback.h>

#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>

#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <Inventor/nodes/SoTexture2.h>
#include <Inventor/nodes/SoTextureUnit.h>
#include <Inventor/nodes/SoBufferedShape.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoVertexShader.h>
#include <Inventor/nodes/SoFragmentShader.h>

#include <Inventor/devices/SoCpuBufferObject.h>
#include <Inventor/SbPlane.h>

#include <LDM/readers/SoVolumeReader.h>
#include <LDM/SoLDMDataAccess.h>

#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoDataRange.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/nodes/SoVolumeBufferedShape.h>
#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>
#include <VolumeViz/readers/SoVRSegyFileReader.h>

#include <MeshVizXLM/mapping/elements/MoMeshElement.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <MeshVizXLM/extractors/MiInterpolatedLogicalSliceExtract.h>

#include <algorithm>

namespace
{
  SoVolumeBufferedShape* createVolumeBufferedShape(const MeXSurfaceMeshUnstructured& extract)
  {
    const MeXGeometryI& geometry = extract.getGeometry();
    const auto& topology = static_cast<const MeXSurfaceTopologyExplicitI&>(extract.getTopology());

    const size_t ni = topology.getNumCells();
    const size_t indicesPerCell = 4;

    auto indexBuffer = new SoCpuBufferObject;
    indexBuffer->setSize(ni * indicesPerCell * sizeof(int32_t));
    int32_t* ib = (int32_t*)indexBuffer->map(SoBufferObject::SET);

    int index = 0;
    for (size_t i = 0; i < ni; ++i)
    {
      auto cell = topology.getCell(i);
      size_t m = cell->getNumNodes();
      assert(m == 4);
      for (size_t j = 0; j < m; ++j)
        ib[index++] = (int32_t)cell->getNodeIndex(j);
    }

    indexBuffer->unmap();

    const size_t floatsPerVertex = 3;
    const size_t stride = floatsPerVertex * sizeof(float);
    const size_t nv = geometry.getSize();

    auto vertexBuffer = new SoCpuBufferObject;
    vertexBuffer->setSize(nv * stride);
    float* vb = (float*)vertexBuffer->map(SoBufferObject::SET);

    index = 0;
    for (size_t i = 0; i < nv; ++i)
    {
      //float x = 5 * (float)i / (float)(nv - 1);

      MbVec3d coord = geometry.getCoord(i);
      vb[index++] = (float)coord[0];
      vb[index++] = (float)coord[1];
      vb[index++] = (float)coord[2];
    }
    vertexBuffer->unmap();

    auto vbs = new SoVolumeBufferedShape;
    vbs->indexBuffer = indexBuffer;
    vbs->indexOffset = 0;
    vbs->indexType = SbDataType::UNSIGNED_INT32;

    vbs->vertexBuffer = vertexBuffer;
    vbs->vertexComponentsCount = 3;
    vbs->vertexComponentsType = SbDataType::FLOAT;
    vbs->vertexOffset = 0;
    vbs->vertexStride = stride;

    vbs->shapeType = SoBufferedShape::QUADS;
    vbs->numVertices = (int32_t)ni * 4;

    return vbs;
  }

  const char* fragGetDataSrc = R"%%%(
  //!oiv_include <VolumeViz/vvizGetData_frag.h>

  VVIZ_DATATYPE VVizGetData(in VVizDataSetId dataset, in vec3 dataCoord)
  {
    if(
      any(lessThan(dataCoord, vec3(0.0, 0.0, 0.0))) ||
      any(greaterThan(dataCoord, vec3(1.0, 1.0, 1.0))))
    {
      return 0.5; // TODO: make this a uniform variable?
    }
    else
    {
      return VVizGetRawData(dataset, dataCoord);
    }
  }
  )%%%";

  const char* fragGetColorSrc = R"%%%(
  //!oiv_include <VolumeViz/vvizGetData_frag.h>           // The shader API lets you include 
  //!oiv_include <VolumeViz/vvizTransferFunction_frag.h>	// declarations of shader functions

  varying in vec3 texcoord;

  uniform VVizDataSetId data1;  // Data texture of 1st volume

  // Method in VolumeViz shader framework to override for custom color computation
  vec4 VVizComputeFragmentColor(VVIZ_DATATYPE vox, vec3 coord)
  {
    VVIZ_DATATYPE value1 = VVizGetData(data1, texcoord);      // Value from 1st volume
    vec4 color1 = VVizTransferFunction(value1, 0);         // Color for 1st volume from TF 0
    return color1;
  }
  )%%%";

  const char* vertexPPSrc= R"%%%(
  varying out vec3 texcoord;

  uniform mat4 texMat;

  void VVizVertexPostProcessing()
  {
    texcoord = (texMat * gl_MultiTexCoord0).xyz;
  }
  )%%%";

  SoVolumeRenderingQuality* createStretchSqueezeVolumeShader(const SbMatrix& texMat)
  {
    auto fragmentShaderData = new SoFragmentShader;
    fragmentShaderData->sourceProgram = fragGetDataSrc;
    fragmentShaderData->sourceType = SoShaderObject::GLSL_PROGRAM;

    auto fragmentShaderColor = new SoFragmentShader;
    fragmentShaderColor->sourceProgram = fragGetColorSrc;
    fragmentShaderColor->sourceType = SoShaderObject::GLSL_PROGRAM;
    fragmentShaderColor->addShaderParameter1i("data1", 1);

    auto vertexShader = new SoVertexShader;
    vertexShader->sourceProgram = vertexPPSrc;
    vertexShader->sourceType = SoShaderObject::GLSL_PROGRAM;
    vertexShader->addShaderParameterMatrix("texMat", texMat);

    auto program = new SoVolumeRenderingQuality;
    program->forVolumeOnly = false; // only doing 2D rendering
    program->shaderObject.set1Value(SoVolumeShader::GET_DATA_FUNCTION, fragmentShaderData);
    program->shaderObject.set1Value(SoVolumeShader::FRAGMENT_COMPUTE_COLOR, fragmentShaderColor);
    program->shaderObject.set1Value(SoVolumeShader::VERTEX_POSTPROCESSING, vertexShader);
    return program;
  }
}

void SeismicScene::createCrossSection(PlaneSlice& slice)
{
  if (!m_mesh)
    return;

  assert(m_presentDayMesh);

  auto const& geometry0 = m_presentDayMesh->getGeometry();
  auto const& topology0 = m_presentDayMesh->getTopology();
  auto const& geometry  = m_mesh->getGeometry();
  auto const& topology  = m_mesh->getTopology();

  double w = m_dimensions.numCellsI * m_dimensions.deltaX;
  double h = m_dimensions.numCellsJ * m_dimensions.deltaY;
  SbBox2d box(0.0, 0.0, w, h);
  SbVec2d p0 = slice.p0;
  SbVec2d p1 = slice.p1;

  if (!clip(p0, p1, box))
    return;

  SbVec2d dir = p1 - p0;
  dir.normalize();
  SbVec3f normal((float)dir[1], (float)-dir[0], 0.f);

  slice.points = computeGridLineIntersections(p0, p1, m_dimensions.deltaX, m_dimensions.deltaY);

  auto dim = m_dimensions;
  auto newEnd = std::remove_if(
    slice.points.begin(), 
    slice.points.end(), 
    [&geometry, &dim](const SbVec2d& p)
    {
      bool ok = true;
      getZ(
        geometry, 
        dim.numCellsI, 
        dim.numCellsJ, 
        p[0] / dim.deltaX, 
        p[1] / dim.deltaY, 0, 
        &ok);

      return !ok;
    });

  slice.points.erase(newEnd, slice.points.end());
  if (slice.points.empty())
    return;

  size_t nk  = topology.getNumCellsK();
  size_t nk0 = topology0.getNumCellsK();
  assert(nk0 >= nk);
  size_t npoints = slice.points.size();
  size_t nverts = npoints * (nk + 1);
  size_t nquads = (npoints - 1) * nk;

  // Init texcoord buffer
  SoCpuBufferObject* tb = new SoCpuBufferObject;
  tb->setSize(nverts * 3 * sizeof(float));
  float* texcoords = (float*)tb->map(SoBufferObject::SET);

  // Init normals buffer
  SoCpuBufferObject* nb = new SoCpuBufferObject;
  nb->setSize(nverts * 3 * sizeof(float));
  float* normals = (float*)nb->map(SoBufferObject::SET);

  // Init vertex buffer
  SoCpuBufferObject* vb = new SoCpuBufferObject;
  vb->setSize(nverts * 3 * sizeof(float));
  float* verts = (float*)vb->map(SoBufferObject::SET);

  for (int k = (int)nk, k0 = (int)nk0; k >= 0; --k, --k0)
  {
    for (auto p : slice.points)
    {
      double i = p[0] / dim.deltaX;
      double j = p[1] / dim.deltaY;
      double z  = getZ(geometry,  dim.numCellsI, dim.numCellsJ, i, j, k);
      double z0 = getZ(geometry0, dim.numCellsI, dim.numCellsJ, i, j, k0);

      *verts++ = (float)p[0];
      *verts++ = (float)p[1];
      *verts++ = (float)z;

      *texcoords++ = (float)p[0];
      *texcoords++ = (float)p[1];
      *texcoords++ = (float)z0;

      *normals++ = normal[0];
      *normals++ = normal[1];
      *normals++ = normal[2];
    }
  }

  vb->unmap();
  tb->unmap();
  nb->unmap();

  // Init index buffer
  SoCpuBufferObject* ib = new SoCpuBufferObject;
  ib->setSize(nquads * 4 * sizeof(uint32_t));
  int* indices = (int*)ib->map(SoBufferObject::SET);
  for (size_t k = 0; k < nk; ++k)
  {
    for (size_t i = 0; i < npoints - 1; ++i)
    {
      uint32_t base = (uint32_t)(k * npoints + i);
      *indices++ = base;
      *indices++ = base + 1;
      *indices++ = (uint32_t)(base + 1 + npoints);
      *indices++ = (uint32_t)(base + npoints);
    }
  }

  ib->unmap();

  auto* shape = new SoVolumeBufferedShape;
  shape->vertexBuffer = vb;
  shape->vertexComponentsCount = 3;
  shape->vertexComponentsType = SbDataType::FLOAT;

  shape->texCoordsBuffer = tb;
  shape->texCoordsComponentsCount = 3;
  shape->texCoordsComponentsType = SbDataType::FLOAT;

  shape->normalBuffer = nb;
  shape->normalComponentsType = SbDataType::FLOAT;

  shape->indexBuffer = ib;
  shape->indexType = SbDataType::UNSIGNED_INT32;
  shape->shapeType = SoBufferedShape::QUADS;
  shape->numVertices = 4 * (int)nquads;
  shape->useNormalsGenerator = false;

  slice.vertices = vb;
  slice.texcoords = tb;
  slice.normals = nb;
  slice.indices = ib;
  slice.shape = shape;
}

void SeismicScene::updateInterpolatedSurface()
{
  if (m_interpolatedSurface.enabled)
  {
    if (!m_interpolatedSurface.extract)
      m_interpolatedSurface.extract.reset(MiInterpolatedLogicalSliceExtract::getNewInstance(*m_mesh));

    if (m_interpolatedSurface.shape)
      m_root->removeChild(m_interpolatedSurface.shape);

    m_interpolatedSurface.extract->extractInterpolatedLogicalSlice(MiMesh::DIMENSION_K, m_interpolatedSurface.position);
    m_interpolatedSurface.shape = createVolumeBufferedShape(m_interpolatedSurface.extract->getExtract());
    m_root->addChild(m_interpolatedSurface.shape);
  }
}

void SeismicScene::updatePlaneSlice(int index)
{
  if (!m_mesh)
    return;

  PlaneSlice& slice = m_planeSlice[index];

  auto extent = m_data->extent.getValue();

  SbVec3f p0, p1;
  if (slice.type == SliceInline)
  {
    p0 = SbVec3f(0.f, slice.position, extent.getMin()[2]);
    p1 = SbVec3f(0.f, slice.position, extent.getMax()[2]);
  }
  else
  {
    p0 = SbVec3f(0.f, extent.getMin()[1], slice.position);
    p1 = SbVec3f(0.f, extent.getMax()[1], slice.position);
  }

  m_seismicTransform.multVecMatrix(p0, p0);
  m_seismicTransform.multVecMatrix(p1, p1);

  slice.p0 = SbVec2d(p0[0], p0[1]);
  slice.p1 = SbVec2d(p1[0], p1[1]);

  // remove previous shape
  if (slice.shape)
  {
    m_root->removeChild(slice.shape);
    slice.shape = nullptr;
  }

  if (slice.enabled)
  {
    if (slice.shape)
    {
      m_root->removeChild(slice.shape);
      slice.shape = nullptr;
    }

    createCrossSection(slice);
    m_root->addChild(slice.shape);
  }
}


SeismicScene::SeismicScene(const char* filename, const Project::Dimensions& dim)
  : m_mesh(nullptr)
  , m_dimensions(dim)
  , m_root(new SoSeparator)
  , m_shader(nullptr)
  , m_transformSeparator(new SoTransformSeparator)
  , m_matrixTransform(new SoMatrixTransform)
  , m_data(new SoVolumeData)
  , m_material(new SoMaterial)
  , m_transferFunction(new SoTransferFunction)
  , m_range(new SoDataRange)
{
  double rangeMin = -10e3, rangeMax = 10e3;

  auto reader = SoVolumeReader::getAppropriateReader(filename);
  m_data->setReader(*reader, true);
  m_data->dataSetId = 1;
  //m_data->getMinMax(rangeMin, rangeMax);

  auto params = new SoLDMResourceParameters;
  params->loadPolicy = SoLDMResourceParameters::ALWAYS;
  params->fixedResolution = true;
  params->resolution = 2;
  m_data->ldmResourceParameters = params;

  m_material->diffuseColor.setValue(SbColor(1.f, 1.f, 1.f));
  m_transferFunction->predefColorMap = SoTransferFunction::BLUE_WHITE_RED;

  m_range->min = rangeMin;
  m_range->max = rangeMax;
  std::cout << "range = { " << rangeMin << " : " << rangeMax << " }" << std::endl;

  // Build scene graph
  m_transformSeparator->addChild(m_matrixTransform);
  m_transformSeparator->addChild(m_data);
  m_transformSeparator->addChild(m_material);
  m_transformSeparator->addChild(m_transferFunction);
  m_transformSeparator->addChild(m_range);
  m_root->addChild(m_transformSeparator);

  SoVRSegyFileReader* segyReader = nullptr;
  if (reader->getTypeId() == SoVRSegyFileReader::getClassTypeId())
  {
    segyReader = static_cast<SoVRSegyFileReader*>(reader);
    //auto header = segyReader->getSegyTextHeader();

    SbVec2d p1, p2, p3, p4;
    segyReader->getP1P2P3Coordinates(p1, p2, p3, p4);
    auto d1 = p2 - p1;
    auto d2 = p3 - p2;

    const float maxDepth = 3722.5f;
    const float minDepth = -1.49f;

    float w = maxDepth - minDepth;
    float h = (float)d1.length();
    float d = (float)d2.length();
    m_data->extent = SbBox3f(0.f, 0.f, 0.f, w, h, d);
    m_normalizeTransform.setScale(SbVec3f(1 / w, 1 / h, 1 / d));

    SbMatrix scaleMatrix;
    scaleMatrix.setScale(SbVec3f(1.f, 1.f, -1.f)); // mirror
    
    SbVec3f xAxis(1.f, 0.f, 0.f);
    SbVec3f yAxis(0.f, 1.f, 0.f);
    SbVec3f zAxis(0.f, 0.f, 1.f);

    SbMatrix rotMatrix1;
    rotMatrix1.setRotate(SbRotation(xAxis, -zAxis));

    d1.normalize();
    SbMatrix rotMatrix2;
    rotMatrix2.setRotate(SbRotation(yAxis, SbVec3f((float)d1[0], (float)d1[1], 0.f)));

    SbMatrix translateMatrix;
    translateMatrix.setTranslate(
      SbVec3f(
        (float)(p1[0] - dim.minX), 
        (float)(p1[1] - dim.minY), 
        -minDepth));

    m_seismicTransform = scaleMatrix * rotMatrix1 * rotMatrix2 * translateMatrix;
    m_invSeismicTransform = m_seismicTransform.inverse();
    m_matrixTransform->matrix = m_seismicTransform;

    // TEMP
    m_shader = createStretchSqueezeVolumeShader(m_invSeismicTransform * m_normalizeTransform);
    m_root->insertChild(m_shader, 0);
  }

  m_planeSlice[0].type = SliceInline;
  m_planeSlice[1].type = SliceCrossline;
}

void SeismicScene::setMesh(const MiVolumeMeshCurvilinear* mesh)
{
  if (mesh != m_mesh)
  {
    m_mesh = mesh;

    if (!m_presentDayMesh)
      m_presentDayMesh.reset(m_mesh->getNewClone());

    for(int i=0; i < 2; ++i)
      updatePlaneSlice(i);
  }
}

SoNode* SeismicScene::getRoot() const
{
  return m_root;
}

SbVec3i32 SeismicScene::getDimensions() const
{
  return m_data->data.getSize();
}

SbBox3f SeismicScene::getExtent() const
{
  return m_data->extent.getValue();
}

void SeismicScene::enableSlice(SliceType type, bool value)
{
  int index = 0;
  switch (type)
  {
  case SliceInline: index = 0; break;
  case SliceCrossline: index = 1; break;
  }

  if (m_planeSlice[index].enabled != value)
  {
    m_planeSlice[index].enabled = value;
    updatePlaneSlice(index);
  }
}

void SeismicScene::setSlicePosition(SliceType type, float position)
{
  int index = 0;
  switch (type)
  {
  case SliceInline: index = 0; break;
  case SliceCrossline: index = 1; break;
  }

  if (m_planeSlice[index].position != position)
  {
    m_planeSlice[index].position = position;
    updatePlaneSlice(index);
  }
}

void SeismicScene::enableInterpolatedSurface(bool enabled)
{
  if (enabled != m_interpolatedSurface.enabled)
  {
    m_interpolatedSurface.enabled = enabled;
    updateInterpolatedSurface();
  }
}

void SeismicScene::setInterpolatedSurfacePosition(float position)
{
  if (position != m_interpolatedSurface.position)
  {
    m_interpolatedSurface.position = position;
    updateInterpolatedSurface();
  }
}
