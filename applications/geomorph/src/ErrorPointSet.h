#ifndef __ErrorPointSet_HH__
#define __ErrorPointSet_HH__

#include <vector>

#include "Numerics.h"

namespace AllochMod {


  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file ErrorPointSet.h
  /// \brief 

  using Numerics::FloatingPoint;

  /// \brief 
  class ErrorPointSet {


  public :

    struct ErrorPosition {
      FloatingPoint error;
      int           i;
      int           j;
    };

    void addError ( const FloatingPoint error,
                    const int           i,
                    const int           j );

    void addError ( const ErrorPosition& p );

    bool hasNearby ( const ErrorPosition& p,
                     const int            iStride,
                     const int            jStride ) const;


    void sortErrors ();

    const ErrorPosition& operator ()( const int i ) const;

    int size () const;

    void clear ();

  private :

    class ErrorPositionOp {

    public :

      bool operator ()( const ErrorPosition& e1,
                        const ErrorPosition& e2 ) const {
        return e1.error < e2.error;
      }

    };

    std::vector<ErrorPosition> errorPositions;

  };

  /** @} */

}

#endif // __ErrorPointSet_HH__
