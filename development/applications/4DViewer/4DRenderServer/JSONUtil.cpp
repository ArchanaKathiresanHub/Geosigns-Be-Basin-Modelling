//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "JSONUtil.h"

#ifdef USE_H264
#include <RenderAreaSettings.h>
#else
#include <RemoteViz/Rendering/RenderAreaSettings.h>
#endif

namespace
{
  template<class T>
  jsonxx::Array toJSON(const std::vector<T>& v)
  {
    jsonxx::Array result;
    for (T x : v)
      result << x;

    return result;
  }

  template<class T>
  jsonxx::Array toJSON(const T* v, int n)
  {
    jsonxx::Array result;
    for(int i=0; i < n; ++i)
      result << v[i];

    return result;
  }

  jsonxx::Array toJSON(const std::vector<SceneGraphManager::FenceParams>& fences)
  {
    jsonxx::Array result;

    for (auto const& f : fences)
    {
      jsonxx::Object obj;
      obj << "id" << f.id;
      obj << "visible" << f.visible;

      result << obj;
    }

    return result;
  }
}

jsonxx::Object toJSON(const RemoteViz::Rendering::RenderAreaSettings& settings)
{
  jsonxx::Object vs;

  vs
    // jpeg quality
    << "stillQuality" << settings.getStillCompressionQuality()
    << "interactiveQuality" << settings.getInteractiveCompressionQuality();

  return vs;
}

jsonxx::Object toJSON(const SeismicScene::ViewState& state)
{
  jsonxx::Object vs;

  vs
    << "seismicInlineSliceEnabled" << state.inlineSliceEnabled
    << "seismicInlineSlicePosition" << state.inlineSlicePosition
    << "seismicCrosslineSliceEnabled" << state.crosslineSliceEnabled
    << "seismicCrosslineSlicePosition" << state.crosslineSlicePosition
    << "seismicInterpolatedSurfaceEnabled" << state.interpolatedSurfaceEnabled
    << "seismicInterpolatedSurfacePosition" << state.interpolatedSurfacePosition
    << "seismicDataRangeMinValue" << state.dataRangeMinValue
    << "seismicDataRangeMaxValue" << state.dataRangeMaxValue;

  return vs;
}

jsonxx::Object toJSON(const SceneGraphManager::ViewState& state)
{
  jsonxx::Object vs;
  vs
    << "currentSnapshotIndex" << state.currentSnapshotIndex
    << "currentPropertyId" << state.currentPropertyId

    // display settings
    << "showFaces" << state.showFaces
    << "showEdges" << state.showEdges
    << "showGrid" << state.showGrid
    << "showCompass" << state.showCompass
    << "showText" << state.showText
    //<< "showPerspective" << (mode == SceneInteractor::PERSPECTIVE)
    << "verticalScale" << state.verticalScale
    << "transparency" << state.transparency

    // traps & flow
    << "showTraps" << state.showTraps
    << "showTrapOutlines" << state.showTrapOutlines
    << "drainageAreaType" << (int)state.drainageAreaType
    << "flowLinesExpulsionStep" << state.flowLinesExpulsionStep
    << "flowLinesLeakageStep" << state.flowLinesLeakageStep
    << "flowLinesExpulsionThreshold" << state.flowLinesExpulsionThreshold
    << "flowLinesLeakageThreshold" << state.flowLinesLeakageThreshold

    // elements
    << "formationVisibility" << toJSON(state.formationVisibility)
    << "surfaceVisibility" << toJSON(state.surfaceVisibility)
    << "reservoirVisibility" << toJSON(state.reservoirVisibility)
    << "faultVisibility" << toJSON(state.faultVisibility)
    << "flowLinesVisibility" << toJSON(state.flowLinesVisibility)

    // slices
    << "slicePosition" << toJSON(state.slicePosition, 3)
    << "sliceEnabled" << toJSON(state.sliceEnabled, 3)

    // fences
    << "fences" << toJSON(state.fences)

    // cell filter
    << "cellFilterEnabled" << state.cellFilterEnabled
    << "cellFilterMinValue" << state.cellFilterMinValue
    << "cellFilterMaxValue" << state.cellFilterMaxValue

    // color scale
    << "colorScaleMapping" << (int)state.colorScaleParams.mapping
    << "colorScaleRange" << (int)state.colorScaleParams.range
    << "colorScaleMinValue" << state.colorScaleParams.minValue
    << "colorScaleMaxValue" << state.colorScaleParams.maxValue;


  return vs;
}

jsonxx::Object toJSON(const SceneGraphManager::PickResult& pickResult)
{
  jsonxx::Object obj;

  std::string typeStr;
  switch (pickResult.type)
  {
  case SceneGraphManager::PickResult::Formation: typeStr = "formation"; break;
  case SceneGraphManager::PickResult::Surface: typeStr = "surface"; break;
  case SceneGraphManager::PickResult::Reservoir: typeStr = "reservoir"; break;
  case SceneGraphManager::PickResult::Trap: typeStr = "trap"; break;
  }

  obj
    << "type" << typeStr
    << "positionX" << pickResult.position[0]
    << "positionY" << pickResult.position[1]
    << "positionZ" << pickResult.position[2]
    << "i" << pickResult.cellIndex[0]
    << "j" << pickResult.cellIndex[1]
    << "k" << pickResult.cellIndex[2]
    << "name" << pickResult.name
    << "propertyValue" << pickResult.propertyValue
    << "trapID" << pickResult.trapID
    << "persistentTrapID" << pickResult.persistentTrapID;

  return obj;
}

jsonxx::Object toJSON(const Project::ProjectInfo& projectInfo)
{
  // Add formation names
  jsonxx::Array formations;
  for (auto formation : projectInfo.formations)
    formations << formation.name;

  // Add surface names
  jsonxx::Array surfaces;
  for (auto surface : projectInfo.surfaces)
    surfaces << surface.name;

  // Add reservoir names
  jsonxx::Array reservoirs;
  for (auto reservoir : projectInfo.reservoirs)
    reservoirs << reservoir.name;

  // Add fault collections
  int collectionId = 0;
  jsonxx::Array faultCollections;
  for (auto faultCollection : projectInfo.faultCollections)
  {
    jsonxx::Array faults;
    for (auto fault : projectInfo.faults)
    {
      if (fault.collectionId == collectionId)
	faults << fault.name;
    }

    jsonxx::Object collection;
    collection << "name" << faultCollection.name;
    collection << "faults" << faults;

    faultCollections << collection;

    collectionId++;
  }

  // Add flow lines
  jsonxx::Array flowlines;
  for (auto lines : projectInfo.flowLines)
    flowlines << lines.name;

  // Add properties
  jsonxx::Array properties;
  for (auto property : projectInfo.properties)
    properties << property.name;

  // Assemble complete projectInfo structure
  jsonxx::Object projectInfoObject;
  projectInfoObject
    << "snapshotCount" << (int)projectInfo.snapshotCount
    << "numI" << projectInfo.dimensions.numCellsI
    << "numJ" << projectInfo.dimensions.numCellsJ
    << "numK" << projectInfo.dimensions.numCellsK
    << "numIHiRes" << projectInfo.dimensions.numCellsIHiRes
    << "numJHiRes" << projectInfo.dimensions.numCellsJHiRes
    << "formations" << formations
    << "surfaces" << surfaces
    << "reservoirs" << reservoirs
    << "faultCollections" << faultCollections
    << "flowLines" << flowlines
    << "properties" << properties;

  return projectInfoObject;
}

jsonxx::Object toJSON(const SeismicScene::Info& info)
{
  jsonxx::Array sizeArray;
  sizeArray
    << info.size[0]
    << info.size[1] 
    << info.size[2];

  jsonxx::Array extentMinArray;
  SbVec3f min = info.extent.getMin();
  extentMinArray << min[0] << min[1] << min[2];

  jsonxx::Array extentMaxArray;
  SbVec3f max = info.extent.getMax();
  extentMaxArray << max[0] << max[1] << max[2];

  jsonxx::Object extentObj;
  extentObj << "min" << extentMinArray;
  extentObj << "max" << extentMaxArray;

  jsonxx::Object seismicInfo;
  seismicInfo << "size" << sizeArray;
  seismicInfo << "extent" << extentObj;

  return seismicInfo;
}
