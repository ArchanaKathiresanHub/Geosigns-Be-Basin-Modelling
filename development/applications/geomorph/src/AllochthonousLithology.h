#ifndef __AllochMod_AllochthonousLithology_HH__
#define __AllochMod_AllochthonousLithology_HH__

#include <iostream>
#include <string>

// DataAccess
#include "Interface/AllochthonousLithology.h"
#include "Interface/ProjectHandle.h"

// TableIO
#include "database.h"

// Application
#include "AllochthonousLithologyDistribution.h"
#include "MapIntervalInterpolator.h"

#include "hdf5.h"

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class Formation;
      class Snapshot;
      class GridMap;
      class Grid;
      class LithoType;
   }
}


namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file AllochthonousLithology.h
  /// \brief Calculates the allochthonous lithology movement for a single formation.

  using namespace DataAccess;
  using Numerics::FloatingPoint;

  /// Extends the DataAccess library class, adding functionality enabling computation and saving of the interpolation.
  class AllochthonousLithology : public Interface::AllochthonousLithology {

    /// \typedef IntervalInterpolatorSequence
    /// \brief The sequence of interval interpolators.
    typedef std::vector <MapIntervalInterpolator*> IntervalInterpolatorSequence;

  public :

    /// \var TimeDimensionScaling
    /// The scaling of the time dimension.
    ///
    /// First the time is scaled to a unit range [0,1], then it is scaled by this factor.
    /// If this number is too large then there will be a gradual dissapearing of the
    /// first map, then some period of no, or very little, salt distribution. Followed
    /// by a gradual phasing-in of the second distribution map. If it is too small then
    /// there will be a rather abrupt change between the distribution maps at some point
    /// around the mid point of the two ages of the maps. 
    static const FloatingPoint TimeDimensionScaling = 0.033;

    /// \var SpaceDimensionScaling
    /// The scaling of the space dimensions.
    ///
    /// First the space is scaled to a unit square [0,1]x[0,1], then it is scaled by this factor.
    static const FloatingPoint SpaceDimensionScaling = 1.0;


    AllochthonousLithology (Interface::ProjectHandle * projectHandle, database::Record * record);

    ~AllochthonousLithology ();


    /// \brief Compute the sequence of interval-interpolants from the sequence of input distribution maps.
    void computeInterpolant ( AllochthonousLithologyDistributionSequence& distributionMaps,
                              const int                                   debugLevel );

    /// \brief Print the boundary, interior and exterior maps to the output stream.
    ///
    /// This is used for debugging purposes only.
    void printDistributionMaps ( AllochthonousLithologyDistributionSequence& inputMaps,
                                 std::ostream&                               output );

    /// \brief Save the interpolation result to file.
    void saveInterpolant ( hid_t& fileId,
                           const std::string& fileName );

  private :

    /// Objects returned from here MUST be de-allocated manually.
    virtual MapIntervalInterpolator* allocateIntervalInterpolator () const;

    /// \brief Compute the transformation vectors mapping each point in the interpolation-point set to the reference domain.
    void computeTransformation ( const AllochthonousLithologyDistribution* lower, 
                                 const AllochthonousLithologyDistribution* upper,
                                       Numerics::GeometryVector&           translation,
                                       Numerics::GeometryVector&           scaling ) const;

    /// \brief The sequence of interval interpolators.
    IntervalInterpolatorSequence interpolators;

  };

  /** @} */

}

#endif // __AllochMod_AllochthonousLithology_HH__
