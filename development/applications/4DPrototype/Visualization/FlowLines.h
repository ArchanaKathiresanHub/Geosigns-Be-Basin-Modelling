#ifndef FLOWLINES_H_INCLUDED
#define FLOWLINES_H_INCLUDED

class SoLineSet;
class SnapshotTopology;
class FlowDirectionProperty;

/**
 * Generate a SoLineSet for a set of flowlines
 * @param values A list of PropertyValue gridmaps for the FlowDirectionIJK property. This list
 *           should not contain null pointers, and should be sorted from top to bottom.
 * @param startK The k-index of the layer where the lines should start
 * @param topology The mesh topology of the snapshot, used for getting the coordinates
 *           of the cell centers, and the overall dimensions of the mesh
 */
SoLineSet* generateFlowLines(
  const FlowDirectionProperty& values,
  int startK, 
  const SnapshotTopology& topology);

#endif
