#ifndef _FASTCAULDRON__PETSC_BLOCK_VECTOR__H_
#define _FASTCAULDRON__PETSC_BLOCK_VECTOR__H_

#include "petsc.h"
#include "PetscVectors.h"

#include "ElementVolumeGrid.h"
#include "NodalVolumeGrid.h"
#include "Mesh3DIndex.h"

template<typename Type>
struct BlockSizeStruct {
   static const int result = Type::BlockSize;
};

template<>
struct BlockSizeStruct<double> {
   static const int result = 1;
};

template<>
struct BlockSizeStruct<const double> {
   static const int result = 1;
};


/// Used to indicate which updating method is required.
enum PETScUpdateMode { NO_UPDATE, UPDATE_EXCLUDING_GHOSTS, UPDATE_INCLUDING_GHOSTS };

/// An interface class giving simplified access to DA global and local 
/// vectors and arrays.
///
/// Intended for arrays containing more than 1 dof per node.
template<typename BlockValueType>
class PetscBlockVector {

public :

   typedef BlockValueType ValueType;

   static const int BlockSize = BlockSizeStruct<BlockValueType>::result;

   /// \brief Default constructor.
   PetscBlockVector ();

   /// \brief Constructor.
   PetscBlockVector ( const ElementVolumeGrid& grid,
                      const Vec                globalVector,
                      const InsertMode         mode,
                      const bool               includeGhosts = false );

   /// \brief Constructor.
   PetscBlockVector ( const NodalVolumeGrid&   grid,
                      const Vec                globalVector,
                      const InsertMode         mode,
                      const bool               includeGhosts = false );

   /// \brief Destructor.
   ~PetscBlockVector ();


   /// \brief initialise the petsc vector.
   ///
   /// The number of dofs in the element grid must match the size of the template parameter.
   void setVector ( const ElementVolumeGrid& grid,
                    const Vec                globalVector,
                    const InsertMode         mode,
                    const bool               includeGhosts = false );

   void setVector ( const NodalVolumeGrid&   grid,
                    const Vec                globalVector,
                    const InsertMode         mode,
                    const bool               includeGhosts = false );

   /// \brief Restore the aray values to the vector.
   ///
   /// This may involve some communicating of values to neighbouring 
   /// processors if ghost values are to be updated.
   void restoreVector ( const PETScUpdateMode updateMode );

   /// \brief Return constant reference to the value-type at the position (k,j,i)
   ///
   /// Notice the indices here, they are 'reversed' from the 'normal' sequence.
   const ValueType& operator ()( const int k, const int j, const int i ) const;

   /// \brief Return reference to the value-type at the position (k,j,i)
   ///
   /// Notice the indices here, they are 'reversed' from the 'normal' sequence.
   ValueType& operator ()( const int k, const int j, const int i );

   /// \brief Return constant reference to the value-type at the index position.
   const ValueType& operator ()( const Mesh3DIndex& index ) const;

   /// \brief Return reference to the value-type at the index position.
   ValueType& operator ()( const Mesh3DIndex& index );


   /// \brief Get the local x-start position. 
   /// 
   /// If include-ghosts was false when setting the vector, then it cannot be true here.
   /// If this is the case it will result in an error.
   int firstI ( const bool includeGhosts = false ) const;

   /// \brief Get the local x-end position. 
   ///
   /// This is a closed interval.
   /// If include-ghosts was false when setting the vector, then it cannot be true here.
   /// If this is the case it will result in an error.
   int lastI ( const bool includeGhosts = false ) const;

   /// \brief Get the local y-start position. 
   /// 
   /// If include-ghosts was false when setting the vector, then it cannot be true here.
   /// If this is the case it will result in an error.
   int firstJ ( const bool includeGhosts = false ) const;

   /// \brief Get the local y-end position. 
   ///
   /// This is a closed interval.
   /// If include-ghosts was false when setting the vector, then it cannot be true here.
   /// If this is the case it will result in an error.
   int lastJ ( const bool includeGhosts = false ) const;

   /// \brief Get the local z-start position. 
   ///
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int firstK () const;

   /// \brief Get the local z-end position. 
   ///
   /// This is a closed interval.
   /// There is no partitioning in the z-direciton, so there can be no ghost nodes.
   int lastK () const;


   // /// Get the element grid used to construct this vector.
   // const ElementVolumeGrid& getGrid () const;


private :

   /// DA from which vector was created.
   DM                 m_da;

   /// The array of values contained within the vector, this can be local or local+ghost values.
   ValueType*** m_values;

   /// Processor local vector.
   Vec          m_localVector;

   /// Vector whose values are to be used.
   Vec          m_globalVector;

   /// Structure containing information.
   DMDALocalInfo  m_localInfo;

   /// Indicates whether or not the array of values has been restored to its vector.
   bool         m_dataRestored;

   /// Indicates whether the vector was retrieved with ghost values or not.
   bool         m_withGhosts;

};

//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//

template<typename BlockValueType>
PetscBlockVector<BlockValueType>::PetscBlockVector () {
   m_da = 0;
   m_values = 0;
   m_localVector = 0;
   m_globalVector = 0;
   m_dataRestored = true;

   m_withGhosts = false;
}

//------------------------------------------------------------//

template<typename BlockValueType>
PetscBlockVector<BlockValueType>::~PetscBlockVector () {

   if ( not m_dataRestored ) {
      DMDAVecRestoreArray ( m_da, m_localVector, &m_values );

      if ( m_globalVector != m_localVector ) {
         //DMRestoreLocalVector ( m_da, &m_localVector );
         Destroy_Petsc_Vector(m_localVector);
      }

   }

   m_da = 0;
   m_values = 0;
   m_localVector = 0;
   m_globalVector = 0;
   m_dataRestored = true;
   m_withGhosts = false;
}

//------------------------------------------------------------//

template<typename BlockValueType>
PetscBlockVector<BlockValueType>::PetscBlockVector ( const ElementVolumeGrid& grid,
                                                     const Vec                globalVector,
                                                     const InsertMode         mode,
                                                     const bool               includeGhosts ) {

   setVector ( grid, globalVector, mode, includeGhosts );
}

//------------------------------------------------------------//

template<typename BlockValueType>
PetscBlockVector<BlockValueType>::PetscBlockVector ( const NodalVolumeGrid& grid,
                                                     const Vec              globalVector,
                                                     const InsertMode       mode,
                                                     const bool             includeGhosts ) {

   setVector ( grid, globalVector, mode, includeGhosts );
}

//------------------------------------------------------------//

template<typename BlockValueType>
void PetscBlockVector<BlockValueType>::setVector ( const ElementVolumeGrid& grid,
                                                   const Vec                globalVector,
                                                   const InsertMode         mode,
                                                   const bool               includeGhosts ) {

   if ( BlockSize != grid.getNumberOfDofs ()) {
      // Error!
      // Is there a way of removing the need for this check?
   }

   m_da = grid.getDa ();
   m_globalVector = globalVector;

   if ( includeGhosts ) {
      //DMGetLocalVector     ( m_da, &m_localVector );
      DMCreateLocalVector(m_da, &m_localVector);
      DMGlobalToLocalBegin ( m_da, m_globalVector, mode, m_localVector );
      DMGlobalToLocalEnd   ( m_da, m_globalVector, mode, m_localVector );
   } else {
      m_localVector = m_globalVector;
   }

   DMDAVecGetArray ( m_da, m_localVector, &m_values );
   m_dataRestored = false;
   DMDAGetLocalInfo ( m_da, &m_localInfo );   

   m_withGhosts = includeGhosts;
}

//------------------------------------------------------------//

template<typename BlockValueType>
void PetscBlockVector<BlockValueType>::setVector ( const NodalVolumeGrid& grid,
                                                   const Vec              globalVector,
                                                   const InsertMode       mode,
                                                   const bool             includeGhosts ) {

   if ( BlockSize != grid.getNumberOfDofs ()) {
      // Error!
      // Is there a way of removing the need for this check?
   }

   m_da = grid.getDa ();
   m_globalVector = globalVector;

   if ( includeGhosts ) {
      //DMGetLocalVector     ( m_da, &m_localVector );
      DMCreateLocalVector(m_da, &m_localVector);
      DMGlobalToLocalBegin ( m_da, m_globalVector, mode, m_localVector );
      DMGlobalToLocalEnd   ( m_da, m_globalVector, mode, m_localVector );
   } else {
      m_localVector = m_globalVector;
   }

   DMDAVecGetArray ( m_da, m_localVector, &m_values );
   m_dataRestored = false;
   DMDAGetLocalInfo ( m_da, &m_localInfo );   

   m_withGhosts = includeGhosts;
}

//------------------------------------------------------------//

template<typename BlockValueType>
void PetscBlockVector<BlockValueType>::restoreVector ( const PETScUpdateMode updateMode ) {

   DMDAVecRestoreArray ( m_da, m_localVector, &m_values );

   if ( m_withGhosts ) {

      switch ( updateMode ) {

         case NO_UPDATE :

            break;
   
         case UPDATE_EXCLUDING_GHOSTS :

            // Should the mode here be the same as that used when setting the global vector?
            // DALocalToGlobal ( m_da, m_localVector, INSERT_VALUES, m_globalVector );
            DMLocalToGlobalBegin ( m_da, m_localVector, INSERT_VALUES, m_globalVector );
            DMLocalToGlobalEnd   ( m_da, m_localVector, INSERT_VALUES, m_globalVector );
            break;

           case UPDATE_INCLUDING_GHOSTS :

              //  Petsc 3.3: Should the ghosted locations be zero in global vector????
              DMLocalToGlobalBegin ( m_da, m_localVector, ADD_VALUES,  m_globalVector );
              DMLocalToGlobalEnd   ( m_da, m_localVector, ADD_VALUES, m_globalVector );
              break;

         default :
      
            PetscPrintf ( PETSC_COMM_WORLD, " WARNING: Update Method not recognized in PETSCBlockVector::restoreGlobalVector\n" );
            break;

      }

   }

   if ( m_withGhosts ) {
      //DMRestoreLocalVector ( m_da, &m_localVector );
      Destroy_Petsc_Vector(m_localVector);
   }

   m_dataRestored = true;
   m_values = 0;
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline const typename PetscBlockVector<BlockValueType>::ValueType& PetscBlockVector<BlockValueType>::operator ()( const int k, const int j, const int i ) const {
   return m_values [ k ][ j ][ i ];
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline typename PetscBlockVector<BlockValueType>::ValueType& PetscBlockVector<BlockValueType>::operator ()( const int k, const int j, const int i ) {
   return m_values [ k ][ j ][ i ];
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline const typename PetscBlockVector<BlockValueType>::ValueType& PetscBlockVector<BlockValueType>::operator ()( const Mesh3DIndex& index ) const {
   return m_values [ index.getK ()][ index.getJ ()][ index.getI ()];
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline typename PetscBlockVector<BlockValueType>::ValueType& PetscBlockVector<BlockValueType>::operator ()( const Mesh3DIndex& index ) {
   return m_values [ index.getK ()][ index.getJ ()][ index.getI ()];
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::firstI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs : m_localInfo.xs );
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::lastI ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gxs + m_localInfo.gxm - 1 : m_localInfo.xs + m_localInfo.xm - 1 );
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::firstJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys : m_localInfo.ys );
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::lastJ ( const bool includeGhosts ) const {
   return ( includeGhosts ? m_localInfo.gys + m_localInfo.gym - 1 : m_localInfo.ys + m_localInfo.ym - 1 );
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::firstK () const {
   return m_localInfo.zs;
}

//------------------------------------------------------------//

template<typename BlockValueType>
inline int PetscBlockVector<BlockValueType>::lastK () const {
   return m_localInfo.zs + m_localInfo.zm - 1;
}

//------------------------------------------------------------//


#endif // _FASTCAULDRON__PETSC_BLOCK_VECTOR__H_
