#ifndef __PrescribedGridPointCondenser_
#define __PrescribedGridPointCondenser_

#include "MapProcessingOperations.h"

namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file PrescribedGridPointCondenser.h
  /// brief 

  /// \brief Mechanism for assigning which points are to be used in the interpolation.
  class PrescribedGridPointCondenser {

  public :

    PrescribedGridPointCondenser ( const int xStart,
                                   const int xEnd,
                                   const int yStart,
                                   const int yEnd );

    /// \brief Fill the map with a single value.
    void fill ( const bool withTheValue );

    /// \brief Return the value at the position defined.
    bool operator () ( const int i, const int j ) const;

    /// \brief Return a reference to the value at the position defined.
    bool& operator () ( const int i, const int j );

  private :

    BinaryMap pointIsRequired;

  };

  /** @} */

}

//------------------------------------------------------------//
// inline functions


inline bool AllochMod::PrescribedGridPointCondenser::operator () ( const int i, const int j ) const {
  return pointIsRequired ( i, j );
}

inline bool& AllochMod::PrescribedGridPointCondenser::operator () ( const int i, const int j ) {
  return pointIsRequired ( i, j );
}

#endif // __PrescribedGridPointCondenser_
