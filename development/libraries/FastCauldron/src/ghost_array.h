#ifndef _GHOST_ARRAY_H_
#define _GHOST_ARRAY_H_

#include <iostream>
#include <iomanip>
using std::ostream;
using std::endl;
using std::setw;

#include "petscdmda.h"

#include "ElementVolumeGrid.h"

enum Ghost_Inclusion { Include_Ghost_Values,
                       Exclude_Ghost_Values };

template <class Type>
class PETSc_Local_2D_Array {

 public:

  PETSc_Local_2D_Array () {

    array1D = 0;
    array2D = 0;

  }

  ~PETSc_Local_2D_Array () {

    if ( array1D != 0 ) {
      delete [] array1D;
      delete [] array2D;
    }

  }

  const Type& operator()( const int I, const int J ) const {
    return array2D [ I - m_localInfo.gxs ][ J - m_localInfo.gys ];
  }

  Type& operator()( const int I, const int J ) {
    return array2D [ I - m_localInfo.gxs ][ J - m_localInfo.gys ];
  }

  void create ( const DM Global_DA ) {
    
    DMDAGetLocalInfo ( Global_DA , &m_localInfo );

    X_Size        = m_localInfo.gxm;
    Y_Size        = m_localInfo.gym;
    Ghost_X_Start = m_localInfo.gxs;
    Ghost_Y_Start = m_localInfo.gys;

    Ghost_Start [ 0 ] = m_localInfo.gxs;
    Ghost_Start [ 1 ] = m_localInfo.gys;

    Ghost_End [ 0 ] = m_localInfo.gxs + m_localInfo.gxm - 1;
    Ghost_End [ 1 ] = m_localInfo.gys + m_localInfo.gym - 1;

    Non_Ghost_Start [ 0 ] = m_localInfo.xs;
    Non_Ghost_Start [ 1 ] = m_localInfo.ys;

    Non_Ghost_End [ 0 ] = m_localInfo.xs + m_localInfo.xm - 1;
    Non_Ghost_End [ 1 ] = m_localInfo.ys + m_localInfo.ym - 1;

    if ( array1D != 0 ) {
       delete [] array1D;
       delete [] array2D;
    }

    array1D = new Type [ m_localInfo.gxm * m_localInfo.gym ];
    array2D = new Type* [ m_localInfo.gxm ];
    
    int I;
    for ( I = 0; I < m_localInfo.gxm; I++ ) array2D [ I ] = &array1D [ I * m_localInfo.gym ];
    
  }

  bool isNull () const {
    return array1D == 0;
  }
  
  void fill ( const Type& Value ) {

    int I;
    for ( I = 0; I < m_localInfo.gxm * m_localInfo.gym; I++ ) array1D [ I ] = Value;

  }

  void Write ( ostream& o ) const {

    int I;
    int J;

    int Width = o.width ();

    for ( I = 0; I < m_localInfo.gxm; I++ ) {

      for ( J = 0; J < m_localInfo.gym; J++ ) {
        o << setw ( Width ) << array2D [ I ][ J ];
      }

      o << endl;
    }

  }


   int firstI ( const bool includeGhosts = false ) const;

   int lastI ( const bool includeGhosts = false ) const;

   int firstJ ( const bool includeGhosts = false ) const;

   int lastJ ( const bool includeGhosts = false ) const;


  // int First ( const int Dimension, const Ghost_Inclusion Inclusion ) const {

  //   if ( Inclusion == Include_Ghost_Values ) {
  //     return Ghost_Start [ Dimension - 1 ];
  //   } else {
  //     return Non_Ghost_Start [ Dimension - 1 ];
  //   }
  // }

  // int Last ( const int Dimension, const Ghost_Inclusion Inclusion ) const {

  //   if ( Inclusion == Include_Ghost_Values ) {
  //     return Ghost_End [ Dimension - 1 ];
  //   } else {
  //     return Non_Ghost_End [ Dimension - 1 ];
  //   }

  // }

 private:

   DMDALocalInfo m_localInfo;

  int X_Size;
  int Y_Size;
  int Ghost_X_Start;
  int Ghost_Y_Start;

  int Ghost_Start [ 2 ];
  int Ghost_End   [ 2 ];

  int Non_Ghost_Start [ 2 ];
  int Non_Ghost_End   [ 2 ];

  Type*  array1D;
  Type** array2D;

};


// ostream& operator<<( ostream& o, const PETSc_Local_2D_Array<double>& Array ) {

//   Array.Write (o);

//   return o;
// }

template <class Type>
class PETSc_Local_3D_Array {

public:

   PETSc_Local_3D_Array ();

   ~PETSc_Local_3D_Array ();


   void create ( const DM Global_DA );

   void create ( const ElementVolumeGrid& grid );

   bool isNull () const;


   const Type& operator()( const int I, const int J, const int K ) const;

   Type& operator()( const int I, const int J, const int K );

   void fill ( const Type& Value );

   PETSc_Local_3D_Array<Type>& operator= ( const PETSc_Local_3D_Array<Type>& anArray );

   int firstI ( const bool includeGhosts = false ) const;

   int lastI ( const bool includeGhosts = false ) const;

   int firstJ ( const bool includeGhosts = false ) const;

   int lastJ ( const bool includeGhosts = false ) const;

   int firstK () const;

   int lastK () const;


private:

   DMDALocalInfo m_localInfo;

   Type*   array1D;
   Type**  array2D;
   Type*** array3D;

};

//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//


template <class Type>
inline int PETSc_Local_2D_Array<Type>::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

template <class Type>
inline int PETSc_Local_2D_Array<Type>::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

template <class Type>
inline int PETSc_Local_2D_Array<Type>::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

template <class Type>
inline int PETSc_Local_2D_Array<Type>::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}



//------------------------------------------------------------//

template <class Type>
PETSc_Local_3D_Array<Type>::PETSc_Local_3D_Array () {
   array1D = 0;
   array2D = 0;
   array3D = 0;
}

template <class Type>
PETSc_Local_3D_Array<Type>::~PETSc_Local_3D_Array () {

   if ( array1D != 0 ) {
      delete [] array1D;
      delete [] array2D;
      delete [] array3D;
   }

}

template <class Type>
const Type& PETSc_Local_3D_Array<Type>::operator()( const int I, const int J, const int K ) const {
   return array3D [ I - m_localInfo.gxs ][ J - m_localInfo.gys ][ K - m_localInfo.gzs ];
}

template <class Type>
Type& PETSc_Local_3D_Array<Type>::operator()( const int I, const int J, const int K ) {
   return array3D [ I - m_localInfo.gxs ][ J - m_localInfo.gys ][ K - m_localInfo.gzs ];
}

template <class Type>
void PETSc_Local_3D_Array<Type>::create ( const DM Global_DA ) {
    
   DMDAGetLocalInfo ( Global_DA , &m_localInfo );

   if ( array1D != 0 ) {
      delete [] array1D;
      delete [] array2D;
      delete [] array3D;
   }

   array1D = new Type   [ m_localInfo.gxm * m_localInfo.gym * m_localInfo.gzm ];
   array2D = new Type*  [ m_localInfo.gxm * m_localInfo.gym ];
   array3D = new Type** [ m_localInfo.gxm ];
   
   int I;
   int Position;

   for ( I = 0; I < m_localInfo.gxm; I++ ) {
      array3D [ I ] = &array2D [ I * m_localInfo.gym ];
   }

   Position = 0;

   for ( I = 0; I < m_localInfo.gxm * m_localInfo.gym; I++ ) {
      array2D [ I ] = &array1D [ Position ];
      Position = Position + m_localInfo.gzm;
   }
    
}

template <class Type>
void PETSc_Local_3D_Array<Type>::create ( const ElementVolumeGrid& grid ) {
   create ( grid.getDa ());
}

template <class Type>
bool PETSc_Local_3D_Array<Type>::isNull () const {
   return array1D == 0;
}
  
template <class Type>
void PETSc_Local_3D_Array<Type>::fill ( const Type& Value ) {

   int i;
   int size = m_localInfo.gxm * m_localInfo.gym * m_localInfo.gzm;

   for ( i = 0; i < size; ++i ) {
      array1D [ i ] = Value;
   }

}

template <class Type>
PETSc_Local_3D_Array<Type>& PETSc_Local_3D_Array<Type>::operator= ( const PETSc_Local_3D_Array<Type>& anArray ) {

   int i;
   int size = m_localInfo.gxm * m_localInfo.gym * m_localInfo.gzm;

   for ( i = 0; i < size; ++i ) {
      array1D [ i ] = anArray.array1D[ i ];
   }
   return * this;
}




template <class Type>
inline int PETSc_Local_3D_Array<Type>::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

template <class Type>
inline int PETSc_Local_3D_Array<Type>::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

template <class Type>
inline int PETSc_Local_3D_Array<Type>::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

template <class Type>
inline int PETSc_Local_3D_Array<Type>::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}

template <class Type>
inline int PETSc_Local_3D_Array<Type>::firstK () const {
   return m_localInfo.zs;
}

template <class Type>
inline int PETSc_Local_3D_Array<Type>::lastK () const {
   return m_localInfo.zs + m_localInfo.zm - 1;
}

#endif /* _GHOST_ARRAY_H_ */
