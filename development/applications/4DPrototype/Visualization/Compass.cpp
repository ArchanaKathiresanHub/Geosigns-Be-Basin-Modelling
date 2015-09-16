#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoFont.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoRotation.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoViewport.h>
#include <Inventor/nodes/SoViewportClipping.h>
#include <Inventor/nodes/SoDepthBuffer.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>

#include <Inventor/elements/SoViewingMatrixElement.h>

#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoGLRenderAction.h>

namespace
{
  SoGroup* createArrows()
  {
    const float cylinderHeight = 1.5f;
    const float cylinderRadius = .1f;
    const float coneHeight = .5f;
    const float coneRadius = .2f;

    SbColor colors[] =
    {
      SbColor(1.f, 0.f, 0.f),
      SbColor(0.f, 1.f, 0.f),
      SbColor(0.f, 0.f, 1.f)
    };

    SbRotation rotations[] =
    {
      SbRotation(SbVec3f(0.f, 0.f, 1.f), (float)-M_PI_2),
      SbRotation(),
      SbRotation(SbVec3f(1.f, 0.f, 0.f), (float)-M_PI_2)
    };

    const char* labels[] =
    {
      "I", "J", "Depth"
    };

    // Make text yellow to stand out from the background
    SoMaterial* textMaterial = new SoMaterial;
    textMaterial->diffuseColor = SbColor(1.f, 1.f, 0.f);

    SoSeparator* root = new SoSeparator;
    SoFont* font = new SoFont;
    font->size = 13.f;
    font->name = "Arial";
    root->addChild(font);

    for (int i = 0; i < 3; ++i)
    {
      SoMaterial* material = new SoMaterial;
      material->diffuseColor = colors[i];

      SoRotation* rotation = new SoRotation;
      rotation->rotation = rotations[i];

      SoTranslation* translation = new SoTranslation;
      translation->translation = SbVec3f(0.f, .5f * cylinderHeight, 0.f);

      SoCylinder* cylinder = new SoCylinder;
      cylinder->height = cylinderHeight;
      cylinder->radius = cylinderRadius;

      float dy = .5f * (coneHeight + cylinderHeight);
      SoTranslation* coneTranslation = new SoTranslation;
      coneTranslation->translation = SbVec3f(0.f, dy, 0.f);

      SoCone* cone = new SoCone;
      cone->height = coneHeight;
      cone->bottomRadius = coneRadius;

      SoTranslation* labelTranslation = new SoTranslation;
      labelTranslation->translation = SbVec3f(0.f, coneHeight, 0.f);

      SoText2* label = new SoText2;
      label->string = labels[i];

      SoTransformSeparator* transformSep = new SoTransformSeparator;
      transformSep->addChild(material);
      transformSep->addChild(rotation);
      transformSep->addChild(translation);
      transformSep->addChild(cylinder);
      transformSep->addChild(coneTranslation);
      transformSep->addChild(cone);
      transformSep->addChild(textMaterial);
      transformSep->addChild(labelTranslation);
      transformSep->addChild(label);

      root->addChild(transformSep);;
    }

    return root;
  }

  void compassCallback(void* userdata, SoAction* action)
  {
    if (!action->isOfType(SoGLRenderAction::getClassTypeId()))
      return;

    SoState* state = action->getState();
    SbMatrix viewMat = SoViewingMatrixElement::get(state);
    SbVec3f translation, scale;
    SbRotation cameraRotation, orient;
    viewMat.getTransform(translation, cameraRotation, scale, orient);

    SoRotation* compassOrientation = reinterpret_cast<SoRotation*>(userdata);
    compassOrientation->rotation = cameraRotation;
  }
}

SoSwitch* createCompass()
{
  const float vpwidth = 200.f;
  const float vpheight = 200.f;

  SoViewport* viewport = new SoViewport;
  viewport->size.setValue(vpwidth, vpheight);

  SoViewportClipping* viewportClipping = new SoViewportClipping;
  viewportClipping->size.setValue(vpwidth, vpheight);

  SoDepthBuffer* depthBuffer = new SoDepthBuffer;
  depthBuffer->clearBuffer = true;

  float c = .5f;
  SbVec3f center(c, c, -c);
  SoTranslation* preTranslation = new SoTranslation;
  preTranslation->translation = -center;
  SoTranslation* postTranslation = new SoTranslation;
  postTranslation->translation = center;
  SoRotation* orientation = new SoRotation;

  SoCallback* callback = new SoCallback;
  callback->setCallback(compassCallback, orientation);

  SoPerspectiveCamera* camera = new SoPerspectiveCamera;

  SoSwitch* root = new SoSwitch;
  root->whichChild = SO_SWITCH_ALL;
  root->boundingBoxIgnoring = true;
  root->addChild(viewport);
  root->addChild(viewportClipping);
  root->addChild(depthBuffer);
  root->addChild(callback);
  root->addChild(camera);

  SoNode* compass = createArrows();
  root->addChild(postTranslation);
  root->addChild(orientation);
  root->addChild(preTranslation);
  root->addChild(compass);

  const float radius = 1.5f;
  SbVec3f r(radius, radius, radius);
  SbBox3f bbox(center - r, center + r);
  SbViewportRegion vpregion((short)vpwidth, (short)vpheight);
  camera->viewAll(bbox, vpregion);

  return root;
}
