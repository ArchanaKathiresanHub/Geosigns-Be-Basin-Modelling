//------------------------------------------------------------//

#ifndef __PointSelectionArbiter_HH__
#define __PointSelectionArbiter_HH__

//------------------------------------------------------------//

#include "Numerics.h"

//------------------------------------------------------------//

namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  using Numerics::FloatingPoint;

  /// \file PointSelectionArbiter.h
  /// \brief Provides

  /// \brief Implementes a mechanism for determining whether, or not, to extract a point.
  ///
  /// The [template parameter] ThresholdGenerator is expected to return a sequence of 
  /// numbers in the range [0,1). The principle is that the ThresholdGenerator, returns
  /// a number and the arbiter checks if this number is less than the upper threshold.
  template <typename ThresholdGenerator>
  class PointSelectionArbiter {

  public :

    /// \brief Initialise an arbiter with an initial threshold, the default
    /// is 0.5 (or 50%).
    PointSelectionArbiter ( const FloatingPoint initialThreshold = 0.5 );

    /// \brief Set an arbiter with a new threshold.
    void setThreshold ( const FloatingPoint newThreshold );

    /// \brief Get current set threshold.
    FloatingPoint getThreshold () const;

    /// \brief Determines whether, or not, to extract a point.
    bool operator ()( const int i,
                      const int j );

  private :

    FloatingPoint      upperThreshold;
    ThresholdGenerator threshold;

  };

  /** @} */

}

//------------------------------------------------------------//

template <typename ThresholdGenerator>
AllochMod::PointSelectionArbiter<ThresholdGenerator>::PointSelectionArbiter ( const FloatingPoint initialThreshold ) {
  upperThreshold = initialThreshold;
}

//------------------------------------------------------------//

template <typename ThresholdGenerator>
void AllochMod::PointSelectionArbiter<ThresholdGenerator>::setThreshold ( const FloatingPoint newThreshold ) {
  upperThreshold = newThreshold;
}

//------------------------------------------------------------//

template <typename ThresholdGenerator>
AllochMod::FloatingPoint AllochMod::PointSelectionArbiter<ThresholdGenerator>::getThreshold () const {
  return upperThreshold;
}

//------------------------------------------------------------//

template <typename ThresholdGenerator>
bool AllochMod::PointSelectionArbiter<ThresholdGenerator>::operator ()( const int i,
                                                                        const int j ) {
  return threshold () < upperThreshold;
}

//------------------------------------------------------------//

#endif // __PointSelectionArbiter_HH__

//------------------------------------------------------------//
