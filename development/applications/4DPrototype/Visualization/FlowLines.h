#ifndef FLOWLINES_H_INCLUDED
#define FLOWLINES_H_INCLUDED

class SoLineSet;
class MiVolumeMeshCurvilinear;
class MiGeometryIjk;
template<class T>
class MiDataSetIjk;
template<class T>
class MbVec3;

#include <memory>
#include <cstdint>
#include <cstddef>

class Project;
template<class T> 
class MiDataSetIj;

MbVec3<double> getCellCenter(const MiGeometryIjk& geometry, size_t i, size_t j, size_t k);

MbVec3<int32_t> decodeFlowDirection(int code);

std::shared_ptr<MiDataSetIj<double> > generateLeakageProperty(const Project& project, size_t snapshotIndex, int reservoirId);

std::shared_ptr<MiDataSetIj<double> > generateExpulsionProperty(const Project& project, size_t snapshotIndex, int formationId);

/**
 * Generate a SoLineSet for a set of flowlines
 * @param values A list of PropertyValue gridmaps for the FlowDirectionIJK property. This list
 *           should not contain null pointers, and should be sorted from top to bottom.
 * @param startK The k-index of the layer where the lines should start
 * @param topology The mesh topology of the snapshot, used for getting the coordinates
 *           of the cell centers, and the overall dimensions of the mesh
 */
SoLineSet* generateFlowLines(
  const MiDataSetIjk<double>& values,
  std::shared_ptr<MiDataSetIj<double> > expulsion,
  const MiVolumeMeshCurvilinear& mesh,
  int startK,
  int step = 1,
  double threshold = 0.0);

#endif
