#include "FormationMapping.h"

FormationMapping::iterator FormationMapping::find ( const LayerProps*& domain ) {
   LayerFindFunctor domFunc ( domain );
   PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   return iterator ( iter );
}

FormationMapping::const_iterator FormationMapping::find ( const LayerProps*& domain ) const {
   LayerFindFunctor domFunc ( domain );
   PairVector::const_iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   return const_iterator ( iter );
}


#if 0
void FormationMapping::insert ( const LayerProps*                    formation,
                                      FormationSubdomainElementGrid* grid ) {

#if 0
   operator[]( formation ) = grid;
#else
   LayerFindFunctor domFunc ( domain );

   PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   if ( iter == m_values.end ()) {
      iter->m_formationGrid = grid;
   } else {

      LayerSortFunctor sortFunc;

      Pair p;
      p.m_formation = formation;
      m_values.push_back ( p );
      std::sort ( m_values.begin (), m_values.end (), sortFunc );

      PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );
      iter->m_formationGrid = grid;
   }
#endif

}
#endif

FormationSubdomainElementGrid*& FormationMapping::operator[]( const LayerProps* domain ) {

   // Check to see if the domain is alread part of the map.
   LayerFindFunctor domFunc ( domain );
   PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

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

   // if ( iter != m_values.end ()) {
   //    // if yes then return reference to the co-domain.
   //    return iter->m_formationGrid;
   // } else {
   //    // add new pair to list, sort it into descending order 
   //    // and return the newly created co-domain.

   //    LayerSortFunctor sortFunc;

   //    Pair p;
   //    p.m_formation = domain;
   //    m_values.push_back ( p );
   //    std::sort ( m_values.begin (), m_values.end (), sortFunc );

   //    // Since the list has been sorted since the pair was added
   //    // we must find the pair in the list again.
   //    PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );
   //    return iter->m_formationGrid;         
   // }


}


   // class iterator {

   // public :

   //    iterator () {
   //    }

   //    iterator ( const PairVectorIterator& init ) {
   //       m_iterator = init;
   //    }

   //    iterator& operator++() {
   //       ++m_iterator;
   //       return *this;
   //    }

   //    Pair& operator*() {
   //       return m_iterator.operator*();
   //    }

   //    Pair* operator->() {
   //       return m_iterator.operator->();
   //    }

   //    iterator& operator=( const iterator& iter ) {
   //       m_iterator = iter.m_iterator;
   //    }

   //    friend bool operator==( const iterator& iter1, const iterator& iter2 ) {
   //       return iter1.m_iterator == iter2.m_iterator;
   //    }

   //    friend bool operator!=( const iterator& iter1, const iterator& iter2 ) {
   //       return not ( iter1 == iter2 );
   //    }

   // private :

   //    PairVectorIterator m_iterator;

   // };

   // class const_iterator {

   // public :

   //    const_iterator () {
   //    }

   //    const_iterator ( const PairVectorConstIterator& init ) {
   //       m_iterator = init;
   //    }

   //    const_iterator& operator++() {
   //       ++m_iterator;
   //       return *this;
   //    }

   //    const Pair& operator*() {
   //       return m_iterator.operator*();
   //    }

   //    const Pair* operator->() {
   //       return m_iterator.operator->();
   //    }

   //    const_iterator& operator=( const const_iterator& iter ) {
   //       m_iterator = iter.m_iterator;
   //    }

   //    friend bool operator==( const const_iterator& iter1, const const_iterator& iter2 ) {
   //       return iter1.m_iterator == iter2.m_iterator;
   //    }

   //    friend bool operator!=( const const_iterator& iter1, const const_iterator& iter2 ) {
   //       return not ( iter1 == iter2 );
   //    }


   // private :

   //    PairVectorConstIterator m_iterator;

   // };

   // class reverse_iterator {

   // public :

   //    reverse_iterator () {
   //    }


   //    reverse_iterator ( const PairVectorReverseIterator& init ) {
   //       m_iterator = init;
   //    }

   //    reverse_iterator& operator++() {
   //       ++m_iterator;
   //       return *this;
   //    }

   //    const Pair* operator*() {
   //       return m_iterator.operator*();
   //    }

   //    Pair* operator->() {
   //       return m_iterator.operator->();
   //    }

   //    reverse_iterator& operator=( const reverse_iterator& iter ) {
   //       m_iterator = iter.m_iterator;
   //    }

   //    friend bool operator==( const reverse_iterator& iter1, const reverse_iterator& iter2 ) {
   //       return iter1.m_iterator == iter2.m_iterator;
   //    }

   //    friend bool operator!=( const reverse_iterator& iter1, const reverse_iterator& iter2 ) {
   //       return not ( iter1 == iter2 );
   //    }

   // private :

   //    PairVectorReverseIterator m_iterator;

   // };

   // class const_reverse_iterator {

   // public :

   //    const_reverse_iterator () {
   //    }

   //    const_reverse_iterator ( const PairVectorConstReverseIterator& init ) {
   //       m_iterator = init;
   //    }

   //    const_reverse_iterator& operator++() {
   //       ++m_iterator;
   //       return *this;
   //    }

   //    const Pair* operator*() {
   //       return m_iterator.operator*();
   //    }

   //    Pair* operator->() {
   //       return m_iterator.operator->();
   //    }

   //    const_reverse_iterator& operator=( const const_reverse_iterator& iter ) {
   //       m_iterator = iter.m_iterator;
   //    }

   //    friend bool operator==( const const_reverse_iterator& iter1, const const_reverse_iterator& iter2 ) {
   //       return iter1.m_iterator == iter2.m_iterator;
   //    }

   //    friend bool operator!=( const const_reverse_iterator& iter1, const const_reverse_iterator& iter2 ) {
   //       return not ( iter1 == iter2 );
   //    }


   // private :

   //    PairVectorConstReverseIterator m_iterator;

   // };

   // void insert ( const LayerProps*                    formation,
   //                     FormationSubdomainElementGrid* grid ) {

   //    LayerFindFunctor domFunc ( domain );

   //    PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );

   //    if ( iter == m_values.end ()) {
   //       iter->m_formationGrid = grid;
   //    } else {

   //       LayerSortFunctor sortFunc;

   //       Pair p;
   //       p.m_formation = formation;
   //       m_values.push_back ( p );
   //       std::sort ( m_values.begin (), m_values.end (), sortFunc );

   //       PairVector::iterator iter = std::find_if ( m_values.begin (), m_values.end (), domFunc );
   //       iter->m_formationGrid = grid;
   //    }

   // }

