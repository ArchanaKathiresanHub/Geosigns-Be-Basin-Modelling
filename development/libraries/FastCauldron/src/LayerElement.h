//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef FASTCAULDRON__VOLUME_ELEMENT__H
#define FASTCAULDRON__VOLUME_ELEMENT__H

#include <string>

#include "GeoPhysicsFluidType.h"

#include "ghost_array.h"

#include "Mesh3DIndex.h"
#include "Nodal3DIndexArray.h"

#include "BoundaryId.h"
#include "ConstantsFastcauldron.h"

/// Some forward declarations of types.
class LayerProps;
class Lithology;


/// Volume element.
///
/// Structure containing information about activity and connectivity of elements 
/// in the domain volumes (e.g. layers and the whole computational domain).
/// If element is not active then only position values are set all other values
/// should be considered undefined.
/// 
///
///
/// Numbers indicate element-node numbers
///
///
///            7              6
///              +----------+ 
///            / :        / |
///          /   :      /   |
///        /     :    /     |
///     4 +----------+ 5    |
///       |      :   |      |
///       |    3 +---|------+ 2
///       |     /    |     / 
///       |   /      |   /
///       | /        | / 
///       +----------+
///      0            1
///
/// Faces are made from a sequence of 4 node numbers:
///     Face_0 : { 0, 1, 2, 3 }
///     Face_1 : { 0, 1, 5, 4 }
///     Face_2 : { 1, 2, 6, 5 }
///     Face_3 : { 2, 3, 7, 6 }
///     Face_4 : { 3, 0, 4, 7 }
///     Face_5 : { 4, 5, 6, 7 }
///
///
/// Normal element configuration is:
///
///            8              7
///              +----------+ 
///            / :        / |
///          /   :      /   |
///        /     :    /     |
///     5 +----------+ 6    |
///       |      :   |      |
///       |    4 +---|------+ 3
///       |     /    |     / 
///       |   /      |   /
///       | /        | / 
///       +----------+
///      1            2
///
///
/// Due to the positive-depth and the k-index counting
/// scheme (upwards, i.e. the opposite direction to the depth)
/// The node enumeration scheme is reflected in the z-direction.
///
///            4              3
///              +----------+ 
///            / :        / |
///          /   :      /   |
///        /     :    /     |
///     1 +----------+ 2    |
///       |      :   |      |
///       |    8 +---|------+ 7
///       |     /    |     / 
///       |   /      |   /
///       | /        | / 
///       +----------+
///      5            6
///
/// Faces are made from a sequence ( should this be a set? is order important?) of 4 node numbers:
///     Face_1 : { 1, 2, 3, 4 }
///     Face_2 : { 1, 2, 6, 5 }
///     Face_3 : { 2, 3, 7, 6 }
///     Face_4 : { 3, 4, 8, 7 }
///     Face_5 : { 4, 1, 5, 8 }
///     Face_6 : { 5, 6, 7, 8 }
///
/// Only the four vertical segments are indicated:
///     Segment_1 : { 1, 5 }
///     Segment_2 : { 2, 6 }
///     Segment_3 : { 3, 7 }
///     Segment_4 : { 4, 8 }
///
///                            _
///                 /\         /|
///                 |        /
///                 |      /
///               +-+--------+
///             / : |      / |
///           /   : |    /   |
///         /     :    /     |
///        +----------+      |
///     <--|...   :   |   ---+-->
///        |      +---|------+
///        |   / /    |     / 
///        | / /    : |   /
///        / /      : | / 
///      / +----------+
///    |_           |
///                 v
///
///
///                 G1       _G4
///                 /\       /|
///                 |       /
///                 |      /
///               +-+--------+
///             / : |      / |
///           /   : |    /   |
///         /     :    /     |
///        +----------+      |
///   G5<--|...   :   |   ---+--> G3
///        |      +---|------+
///        |   / /    |     / 
///        | / /    : |   /
///        / /      : | / 
///      / +----------+
///    |_           |
///   G2            v
///                G6
///
///
///
///             +------------+
///           / :           /|
///         /   :         /  |
///       /     :       /    |
///     /       :     /      |
///    +------------+        |
///    |        :   |        |
///    |        +---|--------+
///    |       /    |       / 
///    |     /      |     / 
///    |   /        |   /
///    | /          | / 
///    +------------+
///
///
///
///               +--------------+
///             / :             /|
///           /   :           /  |
///         /     :         /    |
///       /       :       /      |
///     /         :     /        |
///    +--------------+          |
///    |          :   |          |
///    |          +...|..........+
///    |         /    |         / 
///    |       /      |       / 
///    |     /        |     / 
///    |   /          |   /
///    | /            | / 
///    +--------------+
///
///
///
/// The position in the array of a volume-element is described
/// by four indices: i, j, local-k and global-k. The local-k is the
/// position within the layer, the global-k is the position within
/// the whole domain.
///
class LayerElement {

public :

   static double faceNormalDirectionScaling ( const VolumeData::BoundaryId id );

   // Should this be removed?
   enum NodeId { NODE_1, NODE_2, NODE_3, NODE_4, NODE_5, NODE_6, NODE_7, NODE_8 };


   /// \brief Constructor.
   LayerElement ();

   /// \brief Copy constructor.
   LayerElement ( const LayerElement& element );


   /// \brief Assigmnent operator.
   LayerElement& operator=( const LayerElement& element );

   /// \brief Return whether or not the element is part of a defined region.
   ///
   /// If element is not active then all other values except element position should be ignored.
   bool isActive () const;

   /// Set whether or not the element is part of a defined region.
   void setIsActive ( const bool value );

   /// \name Element position.
   //@{

   /// \brief Return the const reference to the position.
   const Mesh3DIndex& getPosition () const;

   /// \brief Get the i-index of the map.
   int getIPosition () const;

   /// \brief Get the j-index of the map.
   int getJPosition () const;

   /// \brief Get the local k-index of the map.
   int getLocalKPosition () const;

   /// \brief Set the local map indicies.
   void setPosition ( const int i,
                      const int j,
                      const int localK );

   //@}

   /// \name Node position.
   //@{

   /// \brief Return a const referecne to the node-postions array.
   const Nodal3DIndexArray& getNodePositions () const;

   /// \brief Return the const reference to the node index.
   const Mesh3DIndex& getNode ( const int node ) const;

   /// \brief Get the i-index of the node.
   ///
   /// \param node The node at which the i-position is requested.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeIPosition ( const int node ) const;

   /// \brief Get the j-index of the node.
   ///
   /// \param node The node at which the j-position is requested.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeJPosition ( const int node ) const;

   /// \brief Get the k-index of the node for the formation in which the element lies.
   ///
   /// \param node The node at which the k-position is requested.
   /// \pre node in range [ 0 .. 7 ]
   int getNodeLocalKPosition ( const int node ) const;

   /// \brief Set the node indices.
   ///
   /// \param node The node at which the k-position is requested.
   /// \pre node in range [ 0 .. 7 ]
   void setNodePosition ( const int node,
                          const int i,
                          const int j,
                          const int localK );

   //@}


   /// \name Boundary indications.
   //@{

   /// \brief Return whether or not the boundary of the element is on the processor boundary.
   ///
   /// The processor boundary is a sub-set of the domain-boundary, i.e. if an element
   /// is not active then even though it may lie on a processsor boundary this will return false.
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ Gamma_1 .. Gamma_6 ]
   bool isOnProcessorBoundary ( const VolumeData::BoundaryId id ) const;

   /// \brief Set whether or not the boundary of the element is on the processor boundary.
   ///
   /// Which element-boundary is indicated by boundary-id.
   /// boundaryId in range [ Gamma_1 .. Gamma_6 ]
   void setIsOnProcessorBoundary ( const VolumeData::BoundaryId id,
                                   const bool       value );


   /// \brief Return whether or not the boundary of element is active.
   ///
   /// Which element-boundary is indicated by boundary-id.
   bool isActiveBoundary ( const VolumeData::BoundaryId id ) const;

   /// \brief Set whether or not the boundary of the element is active.
   ///
   /// Which element-boundary is indicated by boundary-id.
   void setIsActiveBoundary ( const VolumeData::BoundaryId id,
                              const bool       value );

   /// \brief Get whether or not the element lies on the current processor.
   ///
   /// If element does not lie on this processor then node and boundary data should be ignored.
   bool isOnProcessor () const;

   /// \brief Set whether or not the element lies on the current processor.
   void setIsOnProcessor ( const bool value );

   //@}


   /// \name Neighbour information.
   //@{

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   const LayerElement* getNeighbour ( const VolumeData::BoundaryId id ) const;

   /// \brief Return the neighbouring element.
   ///
   /// May return null if element does not exist.
   /// If lateral face has measure zero then a null will be returned.
   /// For vertical faces (i.e. shallow and deep) then some iteration 
   /// may be required to find an active element, if none is found then
   /// null will be returned.
   const LayerElement* getActiveNeighbour ( const VolumeData::BoundaryId id ) const;

   /// \brief Set the neighbour on the indicated boundary.
   void setNeighbour ( const VolumeData::BoundaryId id,
                       const LayerElement*          neighbour );

   //@}

   /// \name Layer and lithology information.
   //@{

   /// \brief Return pointer to element's lithology.
   const Lithology* getLithology () const;

   /// \brief Set element's lithology.
   void setLithology ( const Lithology* lithology );

   /// \brief Return pointer to element's fluid.
   const GeoPhysics::FluidType* getFluid () const;


   /// \brief Return pointer to formation in which element lives.
   const LayerProps* getFormation () const;

   /// Set formation in which element lives.
   void setFormation ( const LayerProps* formation );

   //@}

   /// \brief Return a string representation of the volume-element position.
   std::string positionImage () const;

   /// \brief Return a string representation of the map-element.
   std::string image () const;

private :

   /// \brief Return the active neighbouring element shallower than current element.
   ///
   /// May return null if element does not exist.
   const LayerElement* getShallowerActiveNeighbour () const;

   /// \brief Return the active neighbouring element deeper than current element.
   ///
   /// May return null if element does not exist.
   const LayerElement* getDeeperActiveNeighbour () const;


   /// \brief Array of node positions.
   Nodal3DIndexArray m_nodePositions;

   /// \brief Element position
   Mesh3DIndex m_arrayPosition;

   /// \brief Array indicating if element boundary lies on processor-partition boundary.
   bool        m_onProcessorBoundary [ VolumeData::NumberOfBoundaries ];

   /// \brief Array indicating if element boundary is active.
   bool        m_activeBoundary [ VolumeData::NumberOfBoundaries ];

   /// \brief Pointers to neighbouring elements across faces.
   const LayerElement* m_neighbours [ VolumeData::NumberOfBoundaries ];

   /// \brief Is the element active or not.
   bool        m_isActive;

   /// \brief Does the element lie in this processor partition or is it a ghost element.
   bool        m_onProcessor;

   /// \brief The formation in which the element lies.
   const LayerProps*  m_formation;

   /// \brief The lithology contained in the element.
   const Lithology*   m_lithology;

}; 


/// \brief A three-d array of volume-elements.
///
/// The size of the array is determined by an element-volume-grid,
/// this may be from a layer or the global domain.
typedef PETSc_Local_3D_Array<LayerElement> LayerElementArray;

/// \brief A three-d array of volume-element pointers.
///
/// The size of the array is determined by an element-volume-grid,
/// this may be from a layer or the global domain.
typedef PETSc_Local_3D_Array<const LayerElement*> LayerElementReferenceArray;


//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//


inline bool LayerElement::isActive () const {
   return m_isActive;
}

inline const Mesh3DIndex& LayerElement::getPosition () const {
   return m_arrayPosition;
}

inline int LayerElement::getIPosition () const {
   return m_arrayPosition.getI ();
}

inline int LayerElement::getJPosition () const {
   return m_arrayPosition.getJ ();
}

inline int LayerElement::getLocalKPosition () const {
   return m_arrayPosition.getK ();
}

inline const Nodal3DIndexArray& LayerElement::getNodePositions () const {
   return m_nodePositions;
}

inline int LayerElement::getNodeIPosition ( const int node ) const {
   return m_nodePositions ( node ).getI ();
}

inline int LayerElement::getNodeJPosition ( const int node ) const {
   return m_nodePositions ( node ).getJ ();
}

inline int LayerElement::getNodeLocalKPosition ( const int node ) const {
   return m_nodePositions ( node ).getK ();
}

inline const Mesh3DIndex& LayerElement::getNode ( const int node ) const {
   return m_nodePositions ( node );
}

inline bool LayerElement::isOnProcessorBoundary ( const VolumeData::BoundaryId id ) const {
   return m_onProcessorBoundary [ id ];
}

inline bool LayerElement::isActiveBoundary ( const VolumeData::BoundaryId id ) const {
   return m_activeBoundary [ id ];
}

inline bool LayerElement::isOnProcessor () const {
   return m_onProcessor;
}

inline const Lithology* LayerElement::getLithology () const {
   return m_lithology;
}

inline const LayerProps* LayerElement::getFormation () const {
   return m_formation;
}

inline const LayerElement* LayerElement::getNeighbour ( const VolumeData::BoundaryId id ) const {
   return m_neighbours [ id ];
}


#endif // FASTCAULDRON__VOLUME_ELEMENT__H
