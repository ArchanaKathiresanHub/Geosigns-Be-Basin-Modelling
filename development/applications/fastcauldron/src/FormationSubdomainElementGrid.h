#ifndef _FASTCAULDRON__FORMATION_SUBDOMAIN_ELEMENT_GRID__H_
#define _FASTCAULDRON__FORMATION_SUBDOMAIN_ELEMENT_GRID__H_

#include "ghost_array.h"
#include "LayerElement.h"
#include "SubdomainElement.h"
#include "BoundaryId.h"

class LayerProps;

/// \brief Class containing subdomain-elements for a formation.
class FormationSubdomainElementGrid {

   /// \brief 3D-array of subdomain-elements.
   typedef PETSc_Local_3D_Array <SubdomainElement> FormationElementArray;

   /// \brief An iterator over elements in the FormationSubdomainElementGrid.
   ///
   /// The template parameter gives the possibility of indicating if  
   /// a particular element is to be included in the set of elements.
   template<class Predicate>
   class GenericElementIterator {

      /// \brief Simple class holding the position in the three-d array.
      struct Index3D {
         int i;
         int j;
         int k;
      };

   public :

      /// \brief Constructor.
      GenericElementIterator ();

      /// \brief Copy constructor.
      GenericElementIterator ( const GenericElementIterator& iter );

      /// \brief Initialise the iterator.
      void initialise ( FormationSubdomainElementGrid* grid,
                        const bool includeGhosts = false );

      /// \brief Get the pointer to the element.
      SubdomainElement* operator->();

      /// \brief Get the reference value of the element.
      SubdomainElement& operator*();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Prefix ++ operator.
      GenericElementIterator& operator++();

      /// \brief Increment the iterator, move to the next element that satisfies the predicate.
      ///
      /// Postfix ++ operator.
      GenericElementIterator operator++( const int dummy );

      /// \brief Increment the iterator by some number of values.
      ///
      /// Move to the nth element that satisfies the predicate.
      GenericElementIterator& operator+=( const int step );

      /// \brief Iterator assignment operator.
      GenericElementIterator& operator=( const GenericElementIterator& rhs );

      /// \brief Determine whether or not the iteration is complete.
      ///
      /// All elements have been iterated over.
      bool isDone () const;

   private :

      /// \brief Advance the positions.
      void advanceCurrent ( bool& finished );

      /// The predicate.
      Predicate m_pred;

      /// The grid over which the iterator is defined.
      FormationSubdomainElementGrid* m_grid;

      /// The start index of the iterator.
      Index3D m_startPosition;

      /// The end index of the iterator.
      Index3D m_endPosition;

      /// The current index of the iterator.
      Index3D m_currentPosition;

      /// \brief Has iteration completed.
      ///
      /// Initialised to true.
      bool    m_complete;

   };


   /// \brief Predicate class 
   class PredicateTrue {
   public :
      /// Returns true irrespective of state of element.
      bool operator ()( const SubdomainElement& element ) const;
   };

   /// \brief Predicate class for active elements.
   class PredicateIsActive {
   public :
      /// Returns true only if element is active.
      bool operator ()( const SubdomainElement& element ) const;
   };

public :

   /// \brief Iterator over all elements in the formation.
   typedef GenericElementIterator<PredicateTrue> ElementIterator;

   /// \brief Iterator over all active elements in the formation.
   typedef GenericElementIterator<PredicateIsActive> ActiveElementIterator;


   /// \brief Constructor.
   FormationSubdomainElementGrid ( LayerProps& layer );

   /// \brief Destructor.
   ~FormationSubdomainElementGrid ();

   /// \brief Initialise the predicated-element-iterator.
   // Should I remove the ability to iterate over the ghost elements?
   template<class Predicate>
   void initialiseIterator ( GenericElementIterator<Predicate>& iter,
                             const bool includeGhosts = false );

   /// \brief Initialise the predicated-element-iterator.
   // Should I remove the ability to iterate over the ghost elements?
   template<class Predicate>
   void initialiseIterator ( GenericElementIterator<Predicate>& iter,
                             const bool includeGhosts = false ) const;


   /// \brief Get whether or not the formation is active.
   bool isActive () const;

   /// \brief Get element at the specified location.
   const SubdomainElement& operator ()( const int i,
                                        const int j,
                                        const int k ) const;

   /// \brief Get element at the specified location.
   SubdomainElement& operator ()( const int i,
                                  const int j,
                                  const int k );


   /// \brief The first index in the x-direction.
   ///
   /// A closed interval [ firstI .. lastI ].
   int firstI ( const bool includeGhosts = false ) const;

   /// \brief The first index in the y-direction.
   ///
   /// A closed interval [ firstJ .. lastJ ].
   int firstJ ( const bool includeGhosts = false ) const;

   /// \brief The first index in the z-direction.
   ///
   /// A closed interval [ firstK .. lastK ].
   /// Counting starts from bottom up, i.e. firstK is the bottom element in the stack of elements.
   /// There are no ghost-elements in the k-direction.
   int firstK () const;


   /// \brief The last index in the x-direction.
   ///
   /// A closed interval [ firstI .. lastI ].
   int lastI ( const bool includeGhosts = false ) const;

   /// \brief The last index in the y-direction.
   ///
   /// A closed interval [ firstJ .. lastJ ].
   int lastJ ( const bool includeGhosts = false ) const;

   /// \brief The last index in the z-direction.
   ///
   ///
   /// A closed interval [ firstK .. lastK ].
   /// Counting starts from bottom up, i.e. lastK is the top element in the stack of elements.
   /// There are no ghost-elements in the k-direction.
   int lastK () const;

   /// \brief The number of items in the I-dimension.
   int lengthI ( const bool includeGhosts = false ) const;

   /// \brief The number of items in the J-dimension.
   int lengthJ ( const bool includeGhosts = false ) const;

   /// \brief The number of items in the K-dimension.
   int lengthK () const;

   /// \brief Get the formation associated with this object.
   LayerProps& getFormation ();

   /// \brief Get the formation associated with this object.
   const LayerProps& getFormation () const;

   /// Create a volume-grid with the number of dofs indicated and add it to the array of volume-grids.
   ///
   /// If volume-grid exists already then no action will be taken.
   void createVolumeGrid ( const int numberOfDofs );

   /// \brief Return reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 );

   /// \brief Return const reference to ElementVolumeGrid with corresponding number of dofs.
   ///
   /// If the corresponding volume-grid does not exist then one will be created.
   const ElementVolumeGrid& getVolumeGrid ( const int numberOfDofs = 1 ) const;

private :


   /// \brief Set the subdomain-elements with the layer-element.
   void copyLayerElements ();

   /// \brief Set the subdomain-element neighbours.
   void linkElements ();


   /// \brief The formation associated with an object.
   LayerProps&           m_formation;

   /// \brief The subdomain-elements for this object.
   FormationElementArray m_elements;

};

//------------------------------------------------------------//

template<class Predicate>
void FormationSubdomainElementGrid::initialiseIterator ( GenericElementIterator<Predicate>& iter,
                                                         const bool includeGhosts ) {
   iter.initialise ( this, includeGhosts );
}

//------------------------------------------------------------//

template<class Predicate>
void FormationSubdomainElementGrid::initialiseIterator ( GenericElementIterator<Predicate>& iter,
                                                         const bool includeGhosts ) const {
   iter.initialise ( this, includeGhosts );
}

// //------------------------------------------------------------//

// inline bool FormationSubdomainElementGrid::isActive () const {
//    return m_formation.isActive ();
// }

//------------------------------------------------------------//

inline SubdomainElement& FormationSubdomainElementGrid::operator ()( const int i,
                                                                     const int j,
                                                                     const int k ) {
   return m_elements ( i, j, k );
}

//------------------------------------------------------------//

inline const SubdomainElement& FormationSubdomainElementGrid::operator ()( const int i,
                                                                           const int j,
                                                                           const int k ) const {
   return m_elements ( i, j, k );
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::firstI ( const bool includeGhosts ) const {
   return m_elements.firstI ( includeGhosts );
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::firstJ ( const bool includeGhosts ) const {
   return m_elements.firstJ ( includeGhosts );
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::firstK () const {
   return m_elements.firstK ();
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::lastI ( const bool includeGhosts ) const {
   return m_elements.lastI ( includeGhosts );
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::lastJ ( const bool includeGhosts ) const {
   return m_elements.lastJ ( includeGhosts );
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::lastK () const {
   return m_elements.lastK ();
}
//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::lengthI ( const bool includeGhosts ) const {
   return lastI ( includeGhosts ) - firstI ( includeGhosts ) + 1;
}

//------------------------------------------------------------//

inline int FormationSubdomainElementGrid::lengthJ ( const bool includeGhosts ) const {
   return lastJ ( includeGhosts ) - firstJ ( includeGhosts ) + 1;
}
//------------------------------------------------------------//


inline int FormationSubdomainElementGrid::lengthK () const {
   return lastK () - firstK () + 1;
}


// //------------------------------------------------------------//

// inline LayerProps& FormationSubdomainElementGrid::getFormation () {
//    return m_formation;
// }

// //------------------------------------------------------------//

// inline const LayerProps& FormationSubdomainElementGrid::getFormation () const {
//    return m_formation;
// }

//------------------------------------------------------------//

inline bool FormationSubdomainElementGrid::PredicateTrue::operator ()( const SubdomainElement& element ) const {
   return true;
}

//------------------------------------------------------------//

inline bool FormationSubdomainElementGrid::PredicateIsActive::operator ()( const SubdomainElement& element ) const {
   return element.getLayerElement ().isActive ();
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::GenericElementIterator () {
   m_grid = 0;
   m_complete = true;
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::GenericElementIterator ( const GenericElementIterator& iter ) {
   m_grid = iter.m_grid;
   m_startPosition = iter.m_startPosition;
   m_endPosition = iter.m_endPosition;
   m_currentPosition = iter.m_currentPosition;
   m_complete = iter.m_complete;
}

//------------------------------------------------------------//

template<class Predicate>
void FormationSubdomainElementGrid::GenericElementIterator<Predicate>::initialise ( FormationSubdomainElementGrid* grid,
                                                                                    const bool includeGhosts ) {

   m_grid = grid;

   m_startPosition.i = m_grid->firstI ( includeGhosts );
   m_startPosition.j = m_grid->firstJ ( includeGhosts );
   m_startPosition.k = m_grid->firstK ();

   m_endPosition.i = m_grid->lastI ( includeGhosts );
   m_endPosition.j = m_grid->lastJ ( includeGhosts );
   m_endPosition.k = m_grid->lastK ();

   m_currentPosition = m_startPosition;
   m_complete = false;

   while ( not m_complete and not m_pred ((*m_grid)( m_currentPosition.i, m_currentPosition.j, m_currentPosition.k ))) {
      advanceCurrent ( m_complete );      
   }

}

//------------------------------------------------------------//

template<class Predicate>
inline SubdomainElement* FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator->() {
   return &(*m_grid)( m_currentPosition.i, m_currentPosition.j, m_currentPosition.k );
}

//------------------------------------------------------------//

template<class Predicate>
inline SubdomainElement& FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator*() {
   return (*m_grid)( m_currentPosition.i, m_currentPosition.j, m_currentPosition.k );
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>&
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator+=( const int step ) {

   int i;

   for ( i = 1; i <= step and not isDone (); ++i ) {
      // call pre-fix ++ operator.
      operator++();
   }

   return *this;
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>&
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator++() {

   if ( not m_complete ) {
      bool finished;

      advanceCurrent ( finished );

      while ( not finished and not m_pred ((*m_grid)( m_currentPosition.i, m_currentPosition.j, m_currentPosition.k ))){
         advanceCurrent ( finished );      
      }

      m_complete = finished;
   }

   return *this;
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator++( const int dummy ) {

   GenericElementIterator copy ( *this );

   ++(*this);

   return copy;
}

//------------------------------------------------------------//

template<class Predicate>
FormationSubdomainElementGrid::GenericElementIterator<Predicate>&
FormationSubdomainElementGrid::GenericElementIterator<Predicate>::operator=( const GenericElementIterator& rhs ) {

   m_grid = rhs.m_grid;
   m_startPosition = rhs.m_startPosition;
   m_endPosition = rhs.m_endPosition;
   m_currentPosition = rhs.m_currentPosition;
   m_complete = rhs.m_complete;

   return *this;
}

//------------------------------------------------------------//

template<class Predicate>
bool FormationSubdomainElementGrid::GenericElementIterator<Predicate>::isDone () const {
   return m_complete;
}

//------------------------------------------------------------//

template<class Predicate>
void FormationSubdomainElementGrid::GenericElementIterator<Predicate>::advanceCurrent ( bool& finished ) {

   finished = false;

   if ( m_currentPosition.k == m_endPosition.k ) {
      m_currentPosition.k = m_startPosition.k;

      if ( m_currentPosition.j == m_endPosition.j ) {
         m_currentPosition.j = m_startPosition.j;

         if ( m_currentPosition.i == m_endPosition.i ) {
            m_currentPosition.i = m_startPosition.i;
            finished = true;
         } else {
            ++m_currentPosition.i;
         }

      } else {
         ++m_currentPosition.j;
      }

   } else {
      ++m_currentPosition.k;
   }

}



#endif // _FASTCAULDRON__FORMATION_SUBDOMAIN_ELEMENT_GRID__H_
