// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_DATASTRUCTUREUTILS_H
#define SUMLIB_DATASTRUCTUREUTILS_H

#include <ostream>
#include <string>
#include <vector>

#include "BaseTypes.h"
#include "SUMlib.h"

namespace SUMlib {

typedef std::vector<std::string> strVector;

typedef std::vector<int> intVector;

/// Writes the contents of an integer vector to string
/// @param [in] s stringstream to write to
/// @param [in] v vector to write
INTERFACE_SUMLIB_DEBUG
std::stringstream& IntVectorToString( std::stringstream& s, intVector const& v );

typedef std::vector< intVector > intMatrix;

/// Writes the contents of an integer matrix to string
/// @param [in] s stringstream to write to
/// @param [in] m matrix to write
INTERFACE_SUMLIB_DEBUG
std::stringstream& IntMatrixToString( std::stringstream& s, intMatrix const& m );

typedef RealVector dblVector;

/// Writes the contents of an double vector to string
/// @param [in] s stringstream to write to
/// @param [in] v vector to write
INTERFACE_SUMLIB_DEBUG
std::stringstream& DblVectorToString( std::stringstream& s, dblVector const& v );

typedef RealMatrix dblMatrix;

/// Writes the contents of an double matrix to string
/// @param [in] s stringstream to write to
/// @param [in] m matrix to write
INTERFACE_SUMLIB_DEBUG
std::stringstream& DblMatrixToString( std::stringstream& s, dblMatrix const& m );

/// Sort the vector of values in ascending order. The index vector contains the
/// indexes of the unsorted elements after sorting.
/// @param [out]     indexes of original elements after sorting
/// @param [in/out]  sorted vector
INTERFACE_SUMLIB_DEBUG
void SelectSort( std::vector<unsigned int>& index, std::vector<double>& values );

/// Find the largest and smallest element in a vector
INTERFACE_SUMLIB_DEBUG
void GetExtremes( dblVector const& vec, double& min, double& max);

/// Return an iterator to an item in the list, specified by its index
INTERFACE_SUMLIB_DEBUG
IndexList::iterator IndexListItem( IndexList const& list, unsigned int index );

/// Determine whether the index list contains a specified index
INTERFACE_SUMLIB_DEBUG
bool IndexListContains( IndexList const& list, unsigned int index );

/// Insert specified index into the index list
INTERFACE_SUMLIB_DEBUG
void IndexListInsert( IndexList& list, unsigned int index );

/// Remove specified index from the index list
INTERFACE_SUMLIB_DEBUG
void IndexListRemove( IndexList& list, unsigned int index );

/// Fill an index list with indices 0..N
INTERFACE_SUMLIB_DEBUG
void IndexListFill( IndexList& list, unsigned int N );

/// Write a vector to a stream
template <typename T>
std::ostream& operator<<( std::ostream& s, std::vector<T> const& v );

/// Write a matrix to a stream
template <typename T>
std::ostream& operator<<( std::ostream& s, std::vector<std::vector<T> > const& m );

/// Map a vector to a smaller vector containing only the elements at positions indicated by the index list.
/// @param [in]  src    vector to select elements from
/// @param [in]  active vector of boolean values, true : element is active, false: element is inactive
/// @param [out] dst    vector of active elements from src
template<typename T>
void MapActive( std::vector<T> const& src, std::vector<bool> const& active, std::vector<T> &dst );

/// Map a vector to a smaller vector containing only the elements at positions indicated by the index list.
/// @param [in]  src    vector to select elements from
/// @param [in]  active vector of indices
/// @param [out] dst    vector of active elements from src
template<typename T>
void MapActive( std::vector<T> const& src, IndexList const& active, std::vector<T> &dst );

} // namespace SUMlib

#include "DataStructureUtils_impl.h"

#endif // SUMLIB_DATASTRUCTUREUTILS_H
