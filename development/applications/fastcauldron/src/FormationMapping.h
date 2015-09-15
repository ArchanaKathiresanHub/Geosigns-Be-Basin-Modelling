#ifndef _FASTCAULDRON__FORMATION_MAPPING__H_
#define _FASTCAULDRON__FORMATION_MAPPING__H_

#include <vector>
#include <algorithm>

#include "layer.h"

/// \brief A simple map type class, mapping formations to formation-grids.
template <class SubdomainGrid>
class FormationMapping {

public :

   /// A {formation, formation-grid}-pair.
   struct Pair {
      const LayerProps* m_formation;
      SubdomainGrid*    m_formationGrid;
   };


   typedef std::vector<Pair> PairVector;


   /// \brief A simple iterator.
   ///
   /// Template parameters : the base-iterator, the item over which the iterator iterates.
   template<class BaseIterator,
            class Iterarand>
   class GenericMapIterator {

   public :

      GenericMapIterator () {}

      GenericMapIterator ( const BaseIterator& iter ) {
         m_iterator = iter;
      }

      GenericMapIterator& operator++() {
         ++m_iterator;
         return *this;
      }

      Iterarand& operator*() {
         return m_iterator.operator*();
      }

      Iterarand* operator->() {
         return m_iterator.operator->();
      }

      GenericMapIterator& operator=( const GenericMapIterator& iter ) {
         m_iterator = iter.m_iterator;
         return *this;
      }

      friend bool operator==( const GenericMapIterator& iter1, const GenericMapIterator& iter2 ) {
         return iter1.m_iterator == iter2.m_iterator;
      }

      friend bool operator!=( const GenericMapIterator& iter1, const GenericMapIterator& iter2 ) {
         return not ( iter1 == iter2 );
      }

   private :

      BaseIterator m_iterator;

   };
      

   /// \brief A simple-iterator instantiation creating a iterator.
   typedef GenericMapIterator <typename PairVector::iterator, Pair>                        iterator;

   /// \brief A simple-iterator instantiation creating a const_iterator.
   typedef GenericMapIterator <typename PairVector::const_iterator, const Pair>            const_iterator;

   /// \brief A simple-iterator instantiation creating a reverse_iterator.
   typedef GenericMapIterator <typename PairVector::reverse_iterator, Pair>                reverse_iterator;

   /// \brief A simple-iterator instantiation creating a const_reverse_iterator.
   typedef GenericMapIterator <typename PairVector::const_reverse_iterator, const Pair>    const_reverse_iterator;

   /// \brief Find the pair containing the formation.
   ///
   /// If the formation is not found then the end iterator will be returned.
   iterator find ( const LayerProps*& domain );

   /// \brief Find the pair containing the formation.
   ///
   /// If the formation is not found then the end iterator will be returned.
   const_iterator find ( const LayerProps*& domain ) const;

   SubdomainGrid*& operator[]( const LayerProps* domain );


   /// \brief Iterator pointing to start of list.
   iterator begin ();

   /// \brief Constant iterator pointing to start of list.
   const_iterator begin () const;

   /// \brief Iterator pointing to one past the end of the list.
   iterator end ();

   /// \brief Constant iterator pointing to one past the end of the list.
   const_iterator end () const;


   /// \brief Reverse iterator pointing to the end of list.
   reverse_iterator rbegin ();

   /// \brief Constant reverse iterator pointing to the end of list.
   const_reverse_iterator rbegin () const;

   /// \brief Reverse iterator pointing to one before the of the list.
   reverse_iterator rend ();

   /// \brief Constant reverse iterator pointing to one before the start of the list.
   const_reverse_iterator rend () const;


private :

   /// \brief Used to find a {formation, formatrion-grid}-pair in the list.
   class LayerFindFunctor {

   public :

      LayerFindFunctor ( const LayerProps*& dom );

      bool operator ()( const Pair& p ) const;

   private :

      const LayerProps*& m_formation;

   };


   /// \brief Used to sort {formation, formatrion-grid}-pairs into descending order of deposition.
   class LayerSortFunctor {

   public :

      bool operator ()( const Pair& p1, const Pair& p2 ) const;

   };

   /// \brief The list of {formation, formation-frid}-pairs.
   PairVector m_values;

};

template <class SubdomainGrid>
inline FormationMapping<SubdomainGrid>::LayerFindFunctor::LayerFindFunctor ( const LayerProps*& dom ) :
   m_formation ( dom ) {
}

template <class SubdomainGrid>
inline bool FormationMapping<SubdomainGrid>::LayerFindFunctor::operator ()( const Pair& p ) const {
   return m_formation == p.m_formation;
}

template <class SubdomainGrid>
inline bool FormationMapping<SubdomainGrid>::LayerSortFunctor::operator ()( const Pair& p1, const Pair& p2 ) const {
   return p1.m_formation->depoage < p2.m_formation->depoage;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::iterator FormationMapping<SubdomainGrid>::begin () {
   iterator iter ( m_values.begin ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::const_iterator FormationMapping<SubdomainGrid>::begin () const {
   const_iterator iter ( m_values.begin ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::iterator FormationMapping<SubdomainGrid>::end () {
   iterator iter ( m_values.end ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::const_iterator FormationMapping<SubdomainGrid>::end () const {
   const_iterator iter ( m_values.end ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::reverse_iterator FormationMapping<SubdomainGrid>::rbegin () {
   reverse_iterator iter ( m_values.rbegin ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::const_reverse_iterator FormationMapping<SubdomainGrid>::rbegin () const {
   const_reverse_iterator iter ( m_values.rbegin ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::reverse_iterator FormationMapping<SubdomainGrid>::rend () {
   reverse_iterator iter ( m_values.rend ());
   return iter;
}

template <class SubdomainGrid>
inline typename FormationMapping<SubdomainGrid>::const_reverse_iterator FormationMapping<SubdomainGrid>::rend () const {
   const_reverse_iterator iter ( m_values.rend ());
   return iter;
}

template <class SubdomainGrid>
typename FormationMapping<SubdomainGrid>::iterator FormationMapping<SubdomainGrid>::find ( const LayerProps*& domain ) {
   LayerFindFunctor domFunc ( domain );
   typename PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   return iterator ( iter );
}

template <class SubdomainGrid>
typename FormationMapping<SubdomainGrid>::const_iterator FormationMapping<SubdomainGrid>::find ( const LayerProps*& domain ) const {
   LayerFindFunctor domFunc ( domain );
   typename PairVector::const_iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   return const_iterator ( iter );
}

template <class SubdomainGrid>
SubdomainGrid*& FormationMapping<SubdomainGrid>::operator[]( const LayerProps* domain ) {

   // Check to see if the domain is alread part of the map.
   LayerFindFunctor domFunc ( domain );
   typename PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   if ( iter == m_values.end ()) {
      // add new pair to list, sort it into descending order 
      // and return the newly created co-domain.

      LayerSortFunctor sortFunc;

      Pair p;

      p.m_formation = domain;
      p.m_formationGrid = 0;

      m_values.push_back ( p );
      // Keep list in descending order of formation deposition.
      std::sort ( m_values.begin (), m_values.end (), sortFunc );

      // Since the list has been sorted since the pair was added
      // we must find the pair in the list again.
      iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );
   }

   return iter->m_formationGrid;
}


#endif // _FASTCAULDRON__FORMATION_MAPPING__H_
