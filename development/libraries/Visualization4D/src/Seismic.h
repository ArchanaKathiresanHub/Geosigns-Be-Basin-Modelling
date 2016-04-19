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
class SoCpuBufferObject;
class SbBox3f;

#include "Project.h"

#include <Inventor/SbVec.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/misc/SoRef.h>

#include <memory>

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

  //struct InterpolatedSurface
  //{
  //  bool enabled = false;
  //  float position = 0.f;

  //  std::shared_ptr<MiInterpolatedLogicalSliceExtract> extract;
  //  SoVolumeBufferedShape* shape = nullptr;
  //} m_interpolatedSurface;

  std::shared_ptr<const MiVolumeMeshCurvilinear> m_presentDayMesh;
  const MiVolumeMeshCurvilinear* m_mesh;

  Project::Dimensions m_dimensions;

  SoSeparator* m_root;
  SoVolumeRenderingQuality* m_shader;
  SoTransformSeparator* m_transformSeparator;
  SoMatrixTransform* m_matrixTransform;

  SoVolumeData* m_data;
  SoMaterial* m_material;
  SoTransferFunction* m_transferFunction;
  SoDataRange* m_range;

  SbMatrix m_seismicTransform;    // align seismic with BPA mesh
  SbMatrix m_invSeismicTransform; // inverse seismic transform
  SbMatrix m_normalizeTransform;  // normalize coords in SoVolumeData::extent range to 0..1

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
};

#endif
