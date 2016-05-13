#ifndef SEISMIC_H_INCLUDED
#define SEISMIC_H_INCLUDED

class SoNode;
class SoGroup;
class SoSeparator;
class SoMatrixTransform;
class SoTransformSeparator;
class SoVolumeData;
class SoMaterial;
class SoTransferFunction;
class SoDataRange;
class SoOrthoSlice;
class SoSwitch;
class MiInterpolatedLogicalSliceExtract;
class MiVolumeMeshCurvilinear;
class SoVolumeBufferedShape;
class SoVolumeRenderingQuality;
class SoVolumeReader;
class SoVRLdmFileReader;
class SoCpuBufferObject;
class SbBox3f;

#include "Project.h"

#include <Inventor/SbVec.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/misc/SoRef.h>

#include <memory>

struct CustomLDMInfo
{
  SbVec2d p1, p2, p3, p4;
  double minDepth;
  double maxDepth;

  void writeXML(FILE* fp);
  void readXML(SoVRLdmFileReader* reader);
};

class SeismicScene
{
public:

  enum SliceType
  {
    SliceInline,
    SliceCrossline
  };

private:

  struct PlaneSlice
  {
    bool enabled = false;
    float position = 0.f;
    SliceType type = SliceInline;

    SbVec2d p0, p1;
    std::vector<SbVec2d> points;

    SoRef<SoCpuBufferObject> vertices;
    SoRef<SoCpuBufferObject> texcoords;
    SoRef<SoCpuBufferObject> normals;
    SoRef<SoCpuBufferObject> indices;

    SoVolumeBufferedShape* shape = nullptr;
  } m_planeSlice[2];

  struct Surface
  {
    bool enabled = false;
    float position = 0.f;

    SoRef<SoCpuBufferObject> vertices;
    SoRef<SoCpuBufferObject> texcoords;
    SoRef<SoCpuBufferObject> indices;

    SoVolumeBufferedShape* shape = nullptr;
  } m_surface;;

  std::shared_ptr<const MiVolumeMeshCurvilinear> m_presentDayMesh;
  const MiVolumeMeshCurvilinear* m_mesh;

  Project::Dimensions m_dimensions;

  SoSeparator* m_root;
  SoSeparator* m_sliceGroup;
  SoSeparator* m_surfaceGroup;

  SoVolumeRenderingQuality* m_sliceShader;
  SoVolumeRenderingQuality* m_surfaceShader;
  SoTransformSeparator* m_transformSeparator;
  SoMatrixTransform* m_matrixTransform;

  SoVolumeData* m_data;
  SoMaterial* m_material;
  SoTransferFunction* m_transferFunction;
  SoDataRange* m_range;

  SbMatrix m_seismicTransform;    // align seismic with BPA mesh
  SbMatrix m_invSeismicTransform; // inverse seismic transform
  SbMatrix m_normalizeTransform;  // normalize coords in SoVolumeData::extent range to 0..1

  void computeVolumeTransform(SoVolumeReader* reader);
    
  void createCrossSection(PlaneSlice& slice);

  void updateInterpolatedSurface();

  void updateSurface();

  void updatePlaneSlice(int index);

public:

  SeismicScene(const char* filename, const Project::Dimensions& dim);

  void setMesh(const MiVolumeMeshCurvilinear* mesh);

  SoNode* getRoot() const;

  SbVec3i32 getDimensions() const;

  SbBox3f getExtent() const;

  void enableSlice(SliceType type, bool value);

  void setSlicePosition(SliceType type, float position);

  void enableInterpolatedSurface(bool enabled);

  void setInterpolatedSurfacePosition(float position);

  void setDataRange(double rangeMin, double rangeMax);

  void loadColorMap(const char* filename);
};

#endif
