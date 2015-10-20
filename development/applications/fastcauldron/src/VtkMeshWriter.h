//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__VTK_MESH_WRITER__H
#define FASTCAULDRON__VTK_MESH_WRITER__H

// Access to STL library.
#include <iostream>
#include <vector>
#include <string>

// Access to fastcauldron application code.
#include "ComputationalDomain.h"

/// \brief Writes the domain to a file in VTK format.
///
/// The legacy VTK file format is used due to its simplicity.
/// The VTK file can be read with paraview.
class VtkMeshWriter {

public :

   /// \brief Simple constructor.
   VtkMeshWriter ();

   /// \brief Write the vtk file of the domain.
   ///
   /// \param [in] domain The domain for which the mesh output is required.
   /// \param [in] fileName The name of the file in which the mesh will be written.
   /// \param [in] zScale   How much to scale the depth.
   /// \param [in] useProjectOrigin Should the project origin be used, if not then (0,0) will be the origin.
   ///
   /// Some project may be very thin when compared to the lateral extent, so in order to 
   /// better display such projects the zScale parameter can scale the depth dimension.
   void save ( const ComputationalDomain& domain,
               const std::string&         fileName,
               const double               zScale = 1.0,
               const bool                 useProjectOrigin = true ) const;

private :

   /// \brief The number of coordinates per point.
   static const int ValuesPerNode = 3;

   typedef std::vector<double> DoubleArray;

   typedef std::vector<int> IntegerArray;


   /// \brief Get the number of active nodes and elements from all processes.
   void getNumberOfActiveNodesAndElements ( const ComputationalDomain& domain,
                                            IntegerArray&              numberOfActiveNodes,
                                            IntegerArray&              numberOfActiveElements ) const;

   /// \brief Get the node positions for the nodes that are local to the process.
   void getLocalNodes ( const ComputationalDomain& domain,
                        DoubleArray&               activeNodes,
                        const double               zScale,
                        const bool                 useProjectOrigin ) const;

   /// \brief Get the element dof numbers for the dofs that are local to the process.
   void getLocalElementDofs ( const ComputationalDomain& domain,
                              IntegerArray&              elementDofs ) const;

   /// \brief Get the element identifier indicating in which layer it lies.
   void getLocalElementLayerIds ( const ComputationalDomain& domain,
                                  IntegerArray&              localElementLayerIds ) const;


   /// \brief Gather the node positions from all processes onto the rank = 0 process.
   void gatherAllNodes ( const DoubleArray&    localNodes,
                         const IntegerArray&   numberOfActiveNodes,
                         DoubleArray&          globalNodes ) const;

   /// \brief Gather the element dof numbers from all processes onto the rank = 0 process.
   void gatherAllElementDofs ( const IntegerArray& localElementDofs,
                               const IntegerArray& numberOfActiveNodeElements,
                               IntegerArray&       globalElementDofs ) const;

   /// \brief Gather the element dof numbers from all processes onto the rank = 0 process.
   void gatherAllElementLayerIds ( const IntegerArray& localElementLayerIds,
                                   const IntegerArray& numberOfActiveElements,
                                   IntegerArray&       globalElementLayerIds ) const;

   /// \brief Perform the write.
   ///
   /// The file is written only on the rank = 0 process.
   void writeFile ( const std::string&  fileName,
                    const int           numberOfNodes,
                    const DoubleArray&  allNodes,
                    const int           numberOfElements,
                    const IntegerArray& allElementDofs,
                    const IntegerArray& allElementLayerIds ) const;

   /// \brief The total number of ranks.
   int m_size;

   /// \brief The rank of this process.
   int m_rank;

};


#endif // FASTCAULDRON__VTK_MESH_WRITER__H
