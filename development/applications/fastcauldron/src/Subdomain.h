//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef _FASTCAULDRON__SUBDOMAIN__H_
#define _FASTCAULDRON__SUBDOMAIN__H_

// STL library.
#include <map>
#include <vector>

// Access to fastcauldron application code.
#include "ElementVolumeGrid.h"
#include "FormationMapping.h"
#include "FormationSubdomainElementGrid.h"
#include "ConstantsFastcauldron.h"
#include "layer.h"
#include "NodalVolumeGrid.h"
#include "StratigraphicColumn.h"
#include "StratigraphicGrids.h"
#include "SubdomainElement.h"


/// \brief Class containing a contiguous subset of the layers in a basin model.
///
/// This may also be a single layer or all possible layers.
class Subdomain {

   /// \typedef FormationToElementGridMap
   /// \brief Mapping from a formation to formation-subdomain-element-grid.
   // Should eventually re-do the subdomain class to use the vector dorectly.
   typedef FormationMapping<FormationSubdomainElementGrid> FormationToElementGridMap;

#if 0
   typedef std::map<const LayerProps*, FormationSubdomainElementGrid*> FormationToElementGridMap;
#endif

   /// \typedef MapIterator
   /// \brief Renaming of iterator.
   typedef FormationToElementGridMap::iterator MapIterator;

   /// \typedef ConstMapIterator
   /// \brief Renaming of const-iterator.
   typedef FormationToElementGridMap::const_iterator ConstMapIterator;

   /// \typedef MapIterator
   /// \brief Renaming of iterator.
   typedef FormationToElementGridMap::reverse_iterator ReverseMapIterator;

   /// \typedef ConstMapIterator
   /// \brief Renaming of const-iterator.
   typedef FormationToElementGridMap::const_reverse_iterator ConstReverseMapIterator;


   /// \brief Iterator over elements in the layers in the subdomain.
   template<class FormationPredicate,
            class FormationIterator,
            class GenericElementIterator>
   class GenericLayerElementIterator {

   public :

      /// \brief Constructor.
      GenericLayerElementIterator ();

      /// \brief Copy constructor.
      GenericLayerElementIterator ( const GenericLayerElementIterator& iter );

      /// \brief Initialise the iterator.
      void initialise ( const FormationIterator& begin,
                        const FormationIterator& end,
                        const bool includeGhosts = false );

      /// \brief Get the pointer to the element.
      SubdomainElement* operator->();

      /// \brief Get the reference value of the element.
      SubdomainElement& operator*();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Prefix ++ operator.
      GenericLayerElementIterator& operator++();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Postfix ++ operator.
      GenericLayerElementIterator operator++( const int dummy );

      /// \brief Increment the iterator by some number of values.
      ///
      /// Move to the nth element that satisfies the predicate.
      GenericLayerElementIterator& operator+=( const int step );

      /// \brief Iterator assignment operator.
      GenericLayerElementIterator& operator=( const GenericLayerElementIterator& rhs );

      /// \brief Determine whether or not the iteration is complete.
      ///
      /// All elements have been iterated over.
      bool isDone () const;

   private :

      /// \brief Advance the iterator to next element.
      ///
      /// This may be in another layer.
      void advanceIterator ();

      /// \brief Advance the layer iterator to next layer.
      void advanceLayerIterator ();


      FormationIterator      m_layerIterator;
      FormationIterator      m_layerIteratorEnd;
      FormationPredicate     m_predicate;
      GenericElementIterator m_layerElementIterator;
      bool                   m_complete;

   };

   /// \brief Iterator over layers in the subdomain.
   template<class FormationPredicate,
            class FormationIterator>
   class GenericLayerIterator {

   public :

      /// \brief Constructor.
      GenericLayerIterator ();

      /// \brief Copy constructor.
      GenericLayerIterator ( const GenericLayerIterator& iter );

      /// \brief Initialise the iterator.
      void initialise ( const FormationIterator& begin,
                        const FormationIterator& end );

      /// \brief Get the pointer to the element.
      // typename FormationIterator::pointer::second_type operator->();
      FormationSubdomainElementGrid* operator->();

      /// \brief Get the reference value of the element.
      FormationSubdomainElementGrid& operator*();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Prefix ++ operator.
      GenericLayerIterator& operator++();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Postfix ++ operator.
      GenericLayerIterator operator++( const int dummy );

      /// \brief Increment the iterator by some number of values.
      ///
      /// Move to the nth element that satisfies the predicate.
      GenericLayerIterator& operator+=( const int step );

      /// \brief Iterator assignment operator.
      GenericLayerIterator& operator=( const GenericLayerIterator& rhs );

      /// \brief Determine whether or not the iteration is complete.
      ///
      /// All elements have been iterated over.
      bool isDone () const;

   private :

      /// \brief Advance the iterator to next layer.
      void advanceIterator ();

      FormationIterator  m_layerIterator;
      FormationIterator  m_layerIteratorEnd;
      FormationPredicate m_predicate;

   };


   /// \brief Always return true whatever the state of the layer.
   class PredicateTrue {
   public :
      bool operator ()( const LayerProps& formation ) const;
   };

   /// \brief Returns true only for active layers.
   class PredicateActiveLayer {
   public :
      bool operator ()( const LayerProps& formation ) const;
   };

   /// \brief Returns true only for active source-rock layers.
   class PredicateActiveSourceRockLayer {
   public :
      bool operator ()( const LayerProps& formation ) const;
   };


public :


   /// \typedef LayerIterator
   /// \brief Iterator over every layer.
   typedef GenericLayerIterator<PredicateTrue, MapIterator> LayerIterator;

   /// \typedef ConstLayerIterator
   /// \brief Iterator over every layer.
   typedef GenericLayerIterator<PredicateTrue, ConstMapIterator> ConstLayerIterator;

   /// \typedef ReverseLayerIterator
   /// \brief Iterator over every layer in reverse order, i.e. from bottom of basement to top.
   typedef GenericLayerIterator<PredicateTrue, ReverseMapIterator> ReverseLayerIterator;

   /// \typedef ConstReverseLayerIterator
   /// \brief Iterator over every layer in reverse order, i.e. from bottom of basement to top.
   typedef GenericLayerIterator<PredicateTrue, ConstReverseMapIterator> ConstReverseLayerIterator;

   /// \typedef ActiveLayerIterator
   /// \brief Iterator over all active layers.
   typedef GenericLayerIterator<PredicateActiveLayer, MapIterator> ActiveLayerIterator;

   /// \typedef ConstActiveLayerIterator
   /// \brief Iterator over all active layers.
   typedef GenericLayerIterator<PredicateActiveLayer, ConstMapIterator> ConstActiveLayerIterator;

   /// \typedef ReverseActiveLayerIterator
   /// \brief Iterator over all active layers in reverse order, i.e. from bottom of basement to top.
   typedef GenericLayerIterator<PredicateActiveLayer, ReverseMapIterator> ReverseActiveLayerIterator;

   /// \typedef ConstReverseActiveLayerIterator
   /// \brief Iterator over all active layers in reverse order, i.e. from bottom of basement to top.
   typedef GenericLayerIterator<PredicateActiveLayer, ConstReverseMapIterator> ConstReverseActiveLayerIterator;

   /// \typedef ActiveSourceRockLayerIterator
   /// \brief Iterator over all active source rock layers.
   typedef GenericLayerIterator<PredicateActiveSourceRockLayer, MapIterator> ActiveSourceRockLayerIterator;

   /// \typedef ConstActiveSourceRockLayerIterator
   /// \brief Iterator over all active source rock layers.
   typedef GenericLayerIterator<PredicateActiveSourceRockLayer, ConstMapIterator> ConstActiveSourceRockLayerIterator;



   /// \typedef ElementIterator
   /// \brief Iterator over every element in every layer.
   typedef GenericLayerElementIterator<PredicateTrue, MapIterator, FormationSubdomainElementGrid::ElementIterator> ElementIterator;

   /// \typedef ActiveLayerAllElementIterator
   /// \brief Iterator over all elements in all active layers.
   typedef GenericLayerElementIterator<PredicateActiveLayer, MapIterator, FormationSubdomainElementGrid::ElementIterator> ActiveLayerAllElementIterator;

   /// \typedef ConstActiveLayerAllElementIterator
   /// \brief Iterator over all elements in all active layers.
   typedef GenericLayerElementIterator<PredicateActiveLayer, ConstMapIterator, FormationSubdomainElementGrid::ElementIterator> ConstActiveLayerAllElementIterator;

   /// \typedef ActiveElementIterator
   /// \brief Iterator over all active elements in all active layers.
   typedef GenericLayerElementIterator<PredicateActiveLayer, MapIterator, FormationSubdomainElementGrid::ActiveElementIterator> ActiveElementIterator;

   /// \typedef ActiveSourceRockElementIterator
   /// \brief Iterator over all active elements in all active source rock layers.
   typedef GenericLayerElementIterator<PredicateActiveSourceRockLayer, MapIterator, FormationSubdomainElementGrid::ActiveElementIterator> ActiveSourceRockElementIterator;



   /// Constructor for a multi-layer subdomain.
   Subdomain ( const LayerProps& topLayer,
               const LayerProps& bottomLayer );

   /// Constructor for a single-layer subdomain.
   Subdomain ( const LayerProps& theLayer );

   /// Destructor.
   virtual ~Subdomain ();


   /// \brief Set the id of the sub-domain.
   void setId ( const int id );

   /// \brief Return the id of the sub-domain.
   int getId () const;

   /// \brief Return whether or not any part of the subdomain is active.
   bool isActive () const;

   /// \brief Return whether or not any source-rock layer in the subdomain is active.
   ///
   /// If the subdomain contains no source rock then this will always be false.
   bool sourceRockIsActive () const;

   /// \brief Indicate whether or not any source-rock contains sulphur.
   bool containsSulphurSourceRock () const;

   /// \brief Set to current time.
   ///
   /// Internal data-structure are updated to the current-time.
   void setActivity ( const double currentTime );

   /// \name Iteration initialisation.
   //@{

   /// \brief Initialise iterator over elements of subdomain.
   template<class Predicate,
            class FormationIterator,
            class GenericElementIterator>
   void initialiseElementIterator ( GenericLayerElementIterator<Predicate, FormationIterator, GenericElementIterator>& iter );

   /// \brief Initialise iterator over elements of subdomain.
   template<class Predicate,
            class FormationIterator,
            class GenericElementIterator>
   void initialiseElementIterator ( GenericLayerElementIterator<Predicate, FormationIterator, GenericElementIterator>& iter ) const;

   /// \brief Intialise iterator over layers of subdomain.
   template<class Predicate,
            class FormationIterator>
   void initialiseLayerIterator ( GenericLayerIterator<Predicate, FormationIterator>& iter );

   /// \brief Intialise iterator over layers of subdomain.
   template<class Predicate,
            class FormationIterator>
   void initialiseLayerIterator ( GenericLayerIterator<Predicate, FormationIterator>& iter ) const;

   /// \brief Intialise iterator over layers of subdomain.
   ///
   /// Specialisation for reverse iterators.
   template<class Predicate>
   void initialiseLayerIterator ( GenericLayerIterator<Predicate, ReverseMapIterator>& iter );

   /// \brief Intialise iterator over layers of subdomain.
   ///
   /// Specialisation for reverse iterators.
   template<class Predicate>
   void initialiseLayerIterator ( GenericLayerIterator<Predicate, ConstReverseMapIterator>& iter ) const;

   //@}

   /// \name .
   //@{

   /// \brief Count the number of elements in depth in active layers.
   int numberOfElements () const;

   /// \brief Count the number of elements in depth in all layers.
   int maximumNumberOfElements () const;

   /// \brief Count the number of node in depth in active layers.
   int numberOfNodes () const;

   /// \brief Count the number of nodes in depth in all layers.
   int maximumNumberOfNodes () const;

   //@}

   /// \brief Return const reference to subdomain-element using subdomain indexing.
   const SubdomainElement& getElement ( const int i,
                                        const int j,
                                        const int k ) const;

   /// \name Subdomain formation access.
   //@{

   /// \brief Determine whether or not a formation is in this subdomain.
   bool hasLayer ( const LayerProps* formation ) const;

   /// \brief Return the element-grid associated with the formation.
   ///
   /// If the formation is not a part of the subdomain then a null value will be returned.
   FormationSubdomainElementGrid* getGrid ( const LayerProps* formation );

   /// \brief Return the element-grid associated with the formation.
   ///
   /// If the formation is not a part of the subdomain then a null value will be returned.
   const FormationSubdomainElementGrid* getGrid ( const LayerProps* formation ) const;

   //@}

   /// \name Grid creation and access.
   //@{

   /// \brief Return reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   const ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 ) const;

   /// \brief Return reference to NodalGrid with corresponding number of dofs.
   ///
   /// If the corresponding node-grid does not exist then one will be created.
   NodalVolumeGrid& getNodeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to NodalGrid with corresponding number of dofs.
   ///
   /// If the corresponding node-grid does not exist then one will be created.
   const NodalVolumeGrid& getNodeGrid ( const int numberOfDofs = 1 ) const;

   //@}

   /// \brief Determine if there is any overlap between two subdomains.
   bool overlaps ( const Subdomain& subdomain ) const;

   //// \brief Return the string representation of the subdomain.
   std::string image () const;

private :


   /// \brief Determine if any source rock formations are active.
   bool determineSourceRockActivity () const;

   /// \brief Initialise the mapping from the global-k index to the layer-pointer and layer-k index.
   void initialiseGlobalKToLayerMapping ();


   /// \brief Set the k-number of the elements in all elements.
   ///
   /// If a layer is active then all of its elements will be numbered, whether they are active or not.
   void numberElements ();

   /// \brief Number the nodal dofs using the current active nodal grid.
   ///
   /// The numbers are for scalar (number-of-dofs=1) grids only.
   void numberNodeDofs ( const double age );

   /// \brief Resize all allocated volume grids with new depth count (element or node).
   void resizeGrids ( const int elementCount,
                      const int nodeCount );

   /// \brief Set the nodal dof numbers.
   ///
   /// Based on a scalar (ndofs = 1) node-grid.
   void numberDofs ();

   /// \brief Add subdomain-element pointers to neighbouring elements across an inter-formation boundary.
   void linkSubdomainElementsVertically ();

   /// \brief Set the top and bottom boundaries of the subdomain.
   ///
   /// Set the top and bottom boundaries in subdomain-elements.
   void setSubdomainBoundary ();

   /// \brief Determine if any source-rock layer contains sulphur.
   bool determineContainsSulphur () const;

   /// \brief The stratigraphic column for this sub-domain.
   StratigraphicColumn               m_column;

   /// \brief Contains the node and elements grids for the current size sub-domain.
   StratigraphicGrids                m_grids;


   /// Is any layer active.
   bool                           m_isActive;

   /// Is any source-rock active.
   bool                           m_sourceRockIsActive;

   /// Does any source-rock contain sulphur.
   bool                           m_containsSulphurSourceRock;

   /// Mapping from layer to formation-element-grid.
   FormationToElementGridMap      m_layerMap;

   /// Current age.
   double                         m_currentAge;

   /// Mapping from subdomain-global-k to layer-k.
   vector<int>                    m_globalKToLayerKMapping;

   /// Mapping from subdomain-global-k to formation-element-grid.
   vector<FormationSubdomainElementGrid*> m_globalKToFormationGridMapping;

   /// \brief 3D array containing the dof numbers for the scalar nodal grid.
   LocalIntegerArray3D               m_scalarDofNumbers;

   /// The maximum width of the stencil.
   // Get better description for this.
   int m_stencilWidth;

   int m_id;

};

//------------------------------------------------------------//
// Some inline functions.
//------------------------------------------------------------//

// inline LayerList& Subdomain::getLayers () {
//    return m_layers;
// }

// //------------------------------------------------------------//

// inline const LayerList& Subdomain::getLayers () const {
//    return m_layers;
// }

//------------------------------------------------------------//

inline const SubdomainElement& Subdomain::getElement ( const int i,
                                                       const int j,
                                                       const int k ) const {
   return m_globalKToFormationGridMapping [ k ]->getElement ( i, j, m_globalKToLayerKMapping [ k ]);
}

//------------------------------------------------------------//

inline int Subdomain::getId () const {
   return m_id;
}

//------------------------------------------------------------//

inline bool Subdomain::isActive () const {
   return m_isActive;
}

//------------------------------------------------------------//

inline bool Subdomain::sourceRockIsActive () const {
   return m_sourceRockIsActive;
}

//------------------------------------------------------------//

inline bool Subdomain::containsSulphurSourceRock () const {
   return m_containsSulphurSourceRock;
}

//------------------------------------------------------------//

template<class Predicate,
         class FormationIterator,
         class GenericElementIterator>
void Subdomain::initialiseElementIterator ( GenericLayerElementIterator<Predicate, FormationIterator, GenericElementIterator>& iter ) {
   iter.initialise ( m_layerMap.begin (), m_layerMap.end ());
}

//------------------------------------------------------------//

template<class Predicate,
         class FormationIterator,
         class GenericElementIterator>
void Subdomain::initialiseElementIterator ( GenericLayerElementIterator<Predicate, FormationIterator, GenericElementIterator>& iter ) const {
   iter.initialise ( m_layerMap.begin (), m_layerMap.end ());
}

//------------------------------------------------------------//

template<class Predicate,
         class FormationIterator>
void Subdomain::initialiseLayerIterator ( GenericLayerIterator<Predicate, FormationIterator>& iter ) {
   iter.initialise ( m_layerMap.begin (), m_layerMap.end ());
}

//------------------------------------------------------------//

template<class Predicate,
         class FormationIterator>
void Subdomain::initialiseLayerIterator ( GenericLayerIterator<Predicate, FormationIterator>& iter ) const {
   iter.initialise ( m_layerMap.begin (), m_layerMap.end ());
}

//------------------------------------------------------------//

template<class Predicate>
void Subdomain::initialiseLayerIterator ( GenericLayerIterator<Predicate, ReverseMapIterator>& iter ) {
   iter.initialise ( m_layerMap.rbegin (), m_layerMap.rend ());
}

//------------------------------------------------------------//

template<class Predicate>
void Subdomain::initialiseLayerIterator ( GenericLayerIterator<Predicate, ConstReverseMapIterator>& iter ) const {
   iter.initialise ( m_layerMap.rbegin (), m_layerMap.rend ());
}

//------------------------------------------------------------//

inline bool Subdomain::PredicateTrue::operator ()( const LayerProps& formation ) const {
   return true;
}

//------------------------------------------------------------//

inline bool Subdomain::PredicateActiveLayer::operator ()( const LayerProps& formation ) const {
   return formation.isActive ();
}

//------------------------------------------------------------//

inline bool Subdomain::PredicateActiveSourceRockLayer::operator ()( const LayerProps& formation ) const {
   return formation.isActive () and formation.isSourceRock ();
}

//------------------------------------------------------------//

#if 0
const SubdomainElement& Subdomain::getElement ( const int i,
                                                const int j,
                                                const int k ) const {

   return m_elements ( i, j, m_globalKToLayerKMapping [ k ]);
}
#endif

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::GenericLayerElementIterator () {
   m_complete = true;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::GenericLayerElementIterator ( const GenericLayerElementIterator& iter ) {
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
void Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::initialise ( const FormationIterator& begin,
                                                                                                                  const FormationIterator& end,
                                                                                                                  const bool includeGhosts ) {

   m_layerIterator = begin;
   m_layerIteratorEnd = end;

   m_complete = false;

   advanceLayerIterator ();

   if ( not m_complete ) {
      m_layerIterator->m_formationGrid->initialiseIterator ( m_layerElementIterator );
   }
   
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
SubdomainElement* Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator->() {
   return m_layerElementIterator.operator->();
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
SubdomainElement& Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator*() {
   return *m_layerElementIterator;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>&
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator++() {

   if ( not m_complete ) {
      advanceIterator ();
   }

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator++( const int dummy ) {

   GenericLayerElementIterator copy ( *this );

   // now advance the iterator.
   operator++();

   return copy;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>&
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator+=( const int step ) {

   int i;

   for ( i = 1; i <= step; ++i ) {
      // Advance iterator by one step.
      operator++();
   }

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>&
Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::operator=( const GenericLayerElementIterator& rhs ) {

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
bool Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::isDone () const {
   return m_complete;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
void Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::advanceIterator () {

   if ( m_layerIterator != m_layerIteratorEnd ) {
      ++m_layerElementIterator;

      if ( m_layerElementIterator.isDone ()) {
         advanceLayerIterator ();

         if ( not m_complete ) {
            m_layerElementIterator.initialise ( m_layerIterator->m_formationGrid );
         }

      }

   }

}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator,
         class ElementIterator>
void Subdomain::GenericLayerElementIterator<FormationPredicate, FormationIterator, ElementIterator>::advanceLayerIterator () {

   if ( m_layerIterator != m_layerIteratorEnd ) {
      // Move to next layer.
      ++m_layerIterator;
   }

   while ( m_layerIterator != m_layerIteratorEnd and not m_predicate ( *m_layerIterator->m_formation )) {
      // Move to next layer that satisifies the predicate.
      ++m_layerIterator;
   }

   m_complete = m_layerIterator == m_layerIteratorEnd;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::GenericLayerIterator () {
   m_layerIterator = m_layerIteratorEnd;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::GenericLayerIterator ( const GenericLayerIterator& iter ) {

}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
void Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::initialise ( const FormationIterator& begin,
                                                                                          const FormationIterator& end ) {

   m_layerIterator = begin;
   m_layerIteratorEnd = end;

   while ( not isDone () and not m_predicate ( *m_layerIterator->m_formation )) {
      ++m_layerIterator;
   }

}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
// typename FormationIterator::pointer::second_type Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator->() {
FormationSubdomainElementGrid* Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator->() {
   return m_layerIterator->m_formationGrid;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
FormationSubdomainElementGrid& Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator*() {
   return *m_layerIterator->m_formationGrid;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>&
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator++() {

   advanceIterator ();

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator++( const int dummy ) {

   GenericLayerIterator<FormationPredicate, FormationIterator> copy ( *this );

   operator++();

   return copy;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>&
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator+=( const int step ) {

   int i;

   for ( i = 1; i <= step; ++i ) {
      advanceIterator ();
   }

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>&
Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::operator=( const GenericLayerIterator& rhs ) {

   m_layerIterator = rhs.m_layerIterator;
   m_layerIteratorEnd = rhs.m_layerIteratorEnd;

   return *this;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
bool Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::isDone () const {
   return m_layerIterator == m_layerIteratorEnd;
}

//------------------------------------------------------------//

template<class FormationPredicate,
         class FormationIterator>
void Subdomain::GenericLayerIterator<FormationPredicate, FormationIterator>::advanceIterator () {

   if ( not isDone ()) {
      ++m_layerIterator;

      while ( not isDone () and not m_predicate ( *m_layerIterator->m_formation )) {
         ++m_layerIterator;
      }

   }

}

//------------------------------------------------------------//

#endif // _FASTCAULDRON__SUBDOMAIN__H_
