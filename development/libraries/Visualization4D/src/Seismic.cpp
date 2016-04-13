#include "Seismic.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoScale.h>

#include <LDM/readers/SoVolumeReader.h>
#include <VolumeViz/nodes/SoVolumeData.h>
#include <VolumeViz/nodes/SoDataRange.h>
#include <VolumeViz/nodes/SoOrthoSlice.h>
#include <VolumeViz/readers/SoVRSegyFileReader.h>

#include <algorithm>

SeismicScene::SeismicScene(const char* filename, double orgX, double orgY)
  : m_root(new SoSeparator)
  , m_data(new SoVolumeData)
  , m_material(new SoMaterial)
  , m_transferFunction(new SoTransferFunction)
  , m_range(new SoDataRange)
  , m_sliceInlineSwitch(new SoSwitch)
  , m_sliceCrosslineSwitch(new SoSwitch)
  , m_sliceDepthSwitch(new SoSwitch)
  , m_sliceInline(new SoOrthoSlice)
  , m_sliceCrossline(new SoOrthoSlice)
  , m_sliceDepth(new SoOrthoSlice)
{
  double rangeMin = 0, rangeMax = 0;

  auto reader = SoVolumeReader::getAppropriateReader(filename);
  m_data->setReader(*reader, true);
  m_data->getMinMax(rangeMin, rangeMax);

  m_volumeDimensions = m_data->data.getSize();

  m_material->diffuseColor.setValue(SbColor(1.f, 1.f, 1.f));
  m_transferFunction->predefColorMap = SoTransferFunction::BLUE_WHITE_RED;

  m_range->min = rangeMin;
  m_range->max = rangeMax;
  std::cout << "range = { " << rangeMin << " : " << rangeMax << " }" << std::endl;

  m_sliceInline->axis = SoOrthoSlice::Y;
  m_sliceInline->sliceNumber = m_volumeDimensions[1] / 2;
  m_sliceInlineSwitch->addChild(m_sliceInline);
  m_sliceInlineSwitch->whichChild = SO_SWITCH_ALL;

  m_sliceCrossline->axis = SoOrthoSlice::Z;
  m_sliceCrossline->sliceNumber = m_volumeDimensions[2] / 2;
  m_sliceCrosslineSwitch->addChild(m_sliceCrossline);
  m_sliceCrosslineSwitch->whichChild = SO_SWITCH_ALL;

  m_sliceDepth->axis = SoOrthoSlice::X;
  m_sliceDepth->sliceNumber = m_volumeDimensions[0] / 2;
  m_sliceDepthSwitch->addChild(m_sliceDepth);
  m_sliceDepthSwitch->whichChild = SO_SWITCH_ALL;

  SoGroup* transforms = new SoGroup;
  SoGroup* slices = new SoGroup;

  // Build scene graph
  m_root->addChild(m_data);
  m_root->addChild(m_material);
  m_root->addChild(m_transferFunction);
  m_root->addChild(m_range);
  m_root->addChild(transforms);
  m_root->addChild(m_sliceInlineSwitch);
  m_root->addChild(m_sliceCrosslineSwitch);
  m_root->addChild(m_sliceDepthSwitch);

  SoVRSegyFileReader* segyReader = nullptr;
  if (reader->getTypeId() == SoVRSegyFileReader::getClassTypeId())
  {
    segyReader = static_cast<SoVRSegyFileReader*>(reader);
    auto header = segyReader->getSegyTextHeader();

    SbVec2d p1, p2, p3, p4;
    segyReader->getP1P2P3Coordinates(p1, p2, p3, p4);
    auto d1 = p2 - p1;
    auto d2 = p3 - p2;

    const float maxDepth = 3722.5f;
    const float minDepth = -1.49f;

    m_data->extent = SbBox3f(0.f, 0.f, 0.f, maxDepth - minDepth, d1.length(), d2.length());

    SoScale* mirror = new SoScale;
    mirror->scaleFactor.setValue(1.f, 1.f, -1.f);

    SoRotation* rot1 = new SoRotation;
    rot1->rotation = SbRotation(SbVec3f(1.f, 0.f, 0.f), SbVec3f(0.f, 0.f, -1.f));

    d1.normalize();
    SoRotation* rot2 = new SoRotation;
    rot2->rotation = SbRotation(SbVec3f(0.f, 1.f, 0.f), SbVec3f(d1[0], d1[1], 0.f));

    SoTranslation* translation2 = new SoTranslation;
    translation2->translation = SbVec3f(p1[0] - orgX, p1[1] - orgY, -minDepth);

    transforms->addChild(translation2);
    transforms->addChild(rot2);
    transforms->addChild(rot1);
    transforms->addChild(mirror);
  }
}

SoNode* SeismicScene::getRoot() const
{
  return m_root;
}

SbVec3i32 SeismicScene::getDimensions() const
{
  return m_volumeDimensions;
}

void SeismicScene::enableSlice(SliceType type, bool value)
{
  auto v = value ? SO_SWITCH_ALL : SO_SWITCH_NONE;

  SoSwitch* node = nullptr;
  switch (type)
  {
  case SliceInline: node = m_sliceInlineSwitch; break;
  case SliceCrossline: node = m_sliceCrosslineSwitch; break;
  case SliceDepth: node = m_sliceDepthSwitch; break;
  }

  node->whichChild = v;
}

void SeismicScene::setSlicePosition(SliceType type, int position)
{
  SoOrthoSlice* node = nullptr;
  switch (type)
  {
  case SliceInline: node = m_sliceInline; break;
  case SliceCrossline: node = m_sliceCrossline; break;
  case SliceDepth: node = m_sliceDepth; break;
  }

  node->sliceNumber = position;
}
