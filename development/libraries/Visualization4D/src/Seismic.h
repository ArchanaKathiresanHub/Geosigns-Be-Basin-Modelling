#ifndef SEISMIC_H_INCLUDED
#define SEISMIC_H_INCLUDED

class SoNode;
class SoSeparator;
class SoVolumeData;
class SoMaterial;
class SoTransferFunction;
class SoDataRange;
class SoOrthoSlice;
class SoSwitch;

#include <Inventor/SbVec.h>

class SeismicScene
{
  SoSeparator* m_root;

  SoVolumeData* m_data;
  SoMaterial* m_material;
  SoTransferFunction* m_transferFunction;
  SoDataRange* m_range;

  SoSwitch* m_sliceInlineSwitch;
  SoSwitch* m_sliceCrosslineSwitch;
  SoSwitch* m_sliceDepthSwitch;

  SoOrthoSlice* m_sliceInline;
  SoOrthoSlice* m_sliceCrossline;
  SoOrthoSlice* m_sliceDepth;

  SbVec3i32 m_volumeDimensions;

public:

  enum SliceType
  {
    SliceInline,
    SliceCrossline,
    SliceDepth
  };

  SeismicScene(const char* filename, double orgX, double orgY);

  SoNode* getRoot() const;

  SbVec3i32 getDimensions() const;

  void enableSlice(SliceType type, bool value);

  void setSlicePosition(SliceType type, int position);
};

#endif
