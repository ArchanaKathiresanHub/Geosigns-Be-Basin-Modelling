#ifndef __AllochthonousLithologyDistribution_HH__
#define __AllochthonousLithologyDistribution_HH__

#include <iostream>
#include <string>

// Common types
#include "Numerics.h"
#include "Point.h"
#include "GenericTwoDArray.h"

// Map processing types
#include "MapProcessingOperations.h"

// DataAccess
#include "GridMap.h"
#include "../../DataAccess/src/AllochthonousLithologyDistribution.h"

// //TableIO
#include "database.h"


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


/*! \defgroup AllochMod Allochthonous Modelling
 *
 */

namespace AllochMod {

  /** @addtogroup AllochMod
   *
   * @{
   */

  /// \file AllochthonousLithologyDistribution.h
  /// \brief Contains the distribution and other associated maps.

  using namespace DataAccess;

  /// \enum MapType
  /// \brief Indicates which map type is required.
  enum MapType { PRIMAL_MAP, /*!< The input map, extended by a single layer of nodes around the map. */
                 DUAL_MAP    /*!< Derived from the primal map, a node at the centre of every element in the primal map */
                              };


  /// \brief Contains the input-map.
  ///
  /// For a give distribution map, this provides:
  ///   - The age of the map;
  ///   - The points that lie on the boundary of the salt body (given in dual mesh coordinates);
  ///   - The points that lie on the outside of the salt body;
  ///   - The points that lie on the inside of the salt body.
  /// several mechanism for getting these points.
  class GeoMorphAllochthonousLithologyDistribution : public Interface::AllochthonousLithologyDistribution {

  public :

    /// \var BoundaryExtensionFactor
    /// \brief How much to extend the boundary of the domain, the factor is used to scale the delta-x and -y.
    ///
    /// The domain is extended beyond that of the input map, so that edge effects of the 
    /// allochthonous lithology distribution can be minimised.
    static constexpr Numerics::FloatingPoint BoundaryExtensionFactor = 5.0;


    GeoMorphAllochthonousLithologyDistribution (Interface::ProjectHandle * projectHandle, database::Record * record);

    ~GeoMorphAllochthonousLithologyDistribution ();

    /// \name Post construction set-up functions.
    /// @{

    // only temporary 
    void inputMap ( const std::string&            fileName,
                    const Numerics::FloatingPoint spaceScaling,
                    const Numerics::FloatingPoint timeScaling );

    /// \brief Loads the distribution grid map from disk, and then initialises the Boolean distribution map.
    void loadGridMap ();


    /// \brief Sets the eroded, dilated, boundary and point-selected maps to their initial state.
    void initialise ();

    /// @}

    /// \name Point removal functions
    /// @{

    /// \brief Pick out \b all boundary points defined in the map, regardless of
    /// whether they have been previously selected or not.
    ///
    /// These points will be taken from the dual map.
    void pickOutAllBoundaryPoints ( Numerics::PointArray& boundaryPoints ) const;

    /// \brief Pick out \b all interior points defined in the map, regardless of
    /// whether they have been previously selected or not.
    ///
    /// The 'primalMapPointHasBeenChosen' member will not be updated ...
    void pickOutAllInteriorPoints ( Numerics::PointArray& interiorPoints ) const;

    /// \brief Pick out \b all exterior points defined in the map, regardless of
    /// whether they have been previously selected or not.
    void pickOutAllExteriorPoints ( Numerics::PointArray& exteriorPoints ) const;



    /// \brief Pick out boundary points that have already been selected from the map.
    void pickOutBoundaryPoints ( Numerics::PointArray& boundaryPoints ) const;

    /// \brief Pick out interior points that have already been selected from the map.
    void pickOutInteriorPoints ( Numerics::PointArray& interiorPoints ) const;

    /// \brief Pick out exterior points that have already been selected from the map.
    void pickOutExteriorPoints ( Numerics::PointArray& exteriorPoints ) const;



    /// \brief Pick out boundary points defined in the map based on the result of the PointSelector.
    ///
    /// The 'primalMapPointHasBeenChosen' member \b will be updated indicating which points were selected.
    template <typename PointSelector>
    void pickOutBoundaryPoints ( PointSelector&        select,
                                 Numerics::PointArray& boundaryPoints );

    /// \brief Pick out interior points defined in the map based on the result of the PointSelector.
    ///
    /// The 'primalMapPointHasBeenChosen' member \b will be updated indicating which points were selected.
    template <typename PointSelector>
    void pickOutInteriorPoints ( PointSelector&        select,
                                 Numerics::PointArray& interiorPoints );

    /// \brief Pick out exterior points defined in the map based on the result of the PointSelector.
    ///
    /// The 'primalMapPointHasBeenChosen' member \b will be updated indicating which points were selected.
    template <typename PointSelector>
    void pickOutExteriorPoints ( PointSelector&        select,
                                 Numerics::PointArray& exteriorPoints );


    /// @}

    /// \name Attribute accessor functions

    /// @{

    /// \brief Get the location in 3d space, (x,y,t), of the point at position (i,j,age).
    ///
    /// This function depends on which map is required.
    Numerics::Point getPoint ( const MapType m,
                               const int     i,
                               const int     j ) const;

    /// \brief Is the point at position (i,j) on the interior of the allochthonous body.
    bool pointIsInterior ( const int i,
                           const int j ) const;

    /// \brief Is the point at position (i,j) on the exterior of the allochthonous body.
    bool pointIsExterior ( const int i,
                           const int j ) const;

    /// \brief Is the point at position (i,j) on the surface of the allochthonous body.
    ///
    /// \warning The subscripts i and j \b must be in the dual grid range.
    bool pointIsOnBoundary ( const int i,
                             const int j ) const;

    /// \brief Has the point at scbscript i, j been selected for use in the interpolation.
    bool pointHasBeenSelected ( const MapType m,
                                const int     i,
                                const int     j ) const;


    /// \brief Return the allochthonous lithology distribution map.
    const AllochMod::BinaryMap& getDistributionMap () const;

    /// \brief Return the \b eroded allochthonous lithology distribution map.
    ///
    /// Explanation of erosion.
    const AllochMod::BinaryMap& getErodedDistributionMap () const;

    /// \brief Return the \b dilated allochthonous lithology distribution map.
    ///
    /// Explanation of dilation.
    const AllochMod::BinaryMap& getDilatedDistributionMap () const;

    /// \brief Return the map containing the boundary of the allochthonous lithology body.
    ///
    /// \warning This map is the dual map.
    const AllochMod::BinaryMap& getBoundaryMap () const;


    /// \brief Return the x-coordinates used in the map.
    ///
    /// This will return the extended x-coordinate range.
    const Numerics::FloatingPointArray& getXCoordinates ( const MapType m = PRIMAL_MAP ) const;

    /// \brief Return the y-coordinates used in the map.
    ///
    /// This will return the extended y-coordinate range.
    const Numerics::FloatingPointArray& getYCoordinates ( const MapType m = PRIMAL_MAP ) const;

    /// \brief Compute and return the number of point that lie inside the allochthonous body for the given map.
    int getNumberOfPointsInsideDomain () const;

    /// \brief Compute and return the number of point that lie outside the allochthonous body for the given map.
    int getNumberOfPointsOutsideDomain () const;

    /// \brief Return the number of points in the x-direction of the original distribution map.
    int mapXDimension () const;

    /// \brief Return the number of points in the y-direction of the original distribution map.
    int mapYDimension () const;

    /// @}

    /// \name Some debugging functions
    /// @{

    void printInteriorMap ( std::ostream& o ) const;

    void printExteriorMap ( std::ostream& o ) const;

    void printBoundaryMap ( std::ostream& o ) const;

    /// @}


  private :

    void setPrimalCoordinates ( const int                           number,
                                const Numerics::FloatingPoint       origin,
                                const Numerics::FloatingPoint       delta,
                                      Numerics::FloatingPointArray& coordinates );

    void setDualCoordinates ( const Numerics::FloatingPointArray& primalCoordinates,
                                    Numerics::FloatingPointArray& dualCoordinates );


    Interface::GridMap* distributionGridMap;
    Numerics::FloatingPoint  age;
    AllochMod::BinaryMap distributionMap;
    AllochMod::BinaryMap erodedDistributionMap;
    AllochMod::BinaryMap dilatedDistributionMap;
    AllochMod::BinaryMap boundaryMap;

    AllochMod::BinaryMap primalMapPointHasBeenChosen;
    AllochMod::BinaryMap dualMapPointHasBeenChosen;

    Numerics::FloatingPointArray primalXCoordinates;
    Numerics::FloatingPointArray primalYCoordinates;
  
    Numerics::FloatingPointArray dualXCoordinates;
    Numerics::FloatingPointArray dualYCoordinates;

    int insideBodyCount;
    int outsideBodyCount;

  };


  /// \typedef AllochthonousLithologyDistributionSequence
  /// \brief A sequence of AllochthonousLithologyDistributions.
  typedef std::vector<GeoMorphAllochthonousLithologyDistribution*> AllochthonousLithologyDistributionSequence;

  /// \brief Functor class determining whether one distribution is older than another.
  class AllochthonousLithologyDistributionGreaterThan {

  public :

    /// Return whether one distribution is older than another.
    bool operator ()( const GeoMorphAllochthonousLithologyDistribution* m1,
                      const GeoMorphAllochthonousLithologyDistribution* m2 ) const;

  };


  /** @} */

}

//------------------------------------------------------------//
//
// Inline functions
//

template <typename PointSelector>
void AllochMod::GeoMorphAllochthonousLithologyDistribution::pickOutBoundaryPoints ( PointSelector&        select,
									    Numerics::PointArray& boundaryPoints ) {


  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = boundaryMap.first ( 1 ); i <= boundaryMap.last ( 1 ); i++ ) {

    for ( j = boundaryMap.first ( 2 ); j <= boundaryMap.last ( 2 ); j++ ) {

      if ( boundaryMap ( i, j ) && select ( i, j ) && ! dualMapPointHasBeenChosen ( i, j )) {
        p ( 0 ) = dualXCoordinates [ i ];
        p ( 1 ) = dualYCoordinates [ j ];
        boundaryPoints.push_back ( p );
        dualMapPointHasBeenChosen ( i, j ) = true;
      }

    }

  }

}

//------------------------------------------------------------//

template <typename PointSelector>
void AllochMod::GeoMorphAllochthonousLithologyDistribution::pickOutInteriorPoints ( PointSelector&        select,
									    Numerics::PointArray& interiorPoints ) {

  int i;
  int j;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = erodedDistributionMap.first ( 1 ); i <= erodedDistributionMap.last ( 1 ); i++ ) {

    for ( j = erodedDistributionMap.first ( 2 ); j <= erodedDistributionMap.last ( 2 ); j++ ) {

      if ( erodedDistributionMap ( i, j ) && select ( i, j ) && ! primalMapPointHasBeenChosen ( i, j )) {
        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        interiorPoints.push_back ( p );
        primalMapPointHasBeenChosen ( i, j ) = true;
      }

    }

  }

}

//------------------------------------------------------------//

template <typename PointSelector>
void AllochMod::GeoMorphAllochthonousLithologyDistribution::pickOutExteriorPoints ( PointSelector&        select,
									    Numerics::PointArray& exteriorPoints ) {

  const int BoundaryCount = 4;
  int i;
  int j;
  bool onSaltBodyBoundary;
  bool onPrimalDomainBoundary;
  bool allEqual;

  Numerics::Point p;

  // The age is the same for all points in the map.
  p ( 2 ) = age;

  for ( i = dilatedDistributionMap.first ( 1 ); i <= dilatedDistributionMap.last ( 1 ); i++ ) {

    for ( j = dilatedDistributionMap.first ( 2 ); j <= dilatedDistributionMap.last ( 2 ); j++ ) {

      if ( i > dilatedDistributionMap.first ( 1 ) && i < dilatedDistributionMap.last ( 1 ) &&
           j > dilatedDistributionMap.first ( 2 ) && j < dilatedDistributionMap.last ( 2 )) {

        allEqual = distributionMap ( i, j ) == distributionMap ( i - 1, j - 1 ) && 
                   distributionMap ( i, j ) == distributionMap ( i - 1, j + 1 ) && 
                   distributionMap ( i, j ) == distributionMap ( i + 1, j + 1 ) && 
                   distributionMap ( i, j ) == distributionMap ( i + 1, j - 1 );

        onSaltBodyBoundary = ! distributionMap ( i, j ) && !allEqual;
      }

      onSaltBodyBoundary = false;

      onPrimalDomainBoundary = ( i == 0 && ( j % BoundaryCount == 0 )) || ( i == dilatedDistributionMap.last ( 1 ) && ( j % BoundaryCount == 0 )) ||
                               ( j == 0 && ( i % BoundaryCount == 0 )) || ( j == dilatedDistributionMap.last ( 2 ) && ( i % BoundaryCount == 0 ));

      onPrimalDomainBoundary = false;

      if (( onPrimalDomainBoundary || onSaltBodyBoundary || 
            (( dilatedDistributionMap ( i, j ) == false ) && select ( i, j ))) && ! primalMapPointHasBeenChosen ( i, j )) {

        p ( 0 ) = primalXCoordinates [ i ];
        p ( 1 ) = primalYCoordinates [ j ];
        exteriorPoints.push_back ( p );
        primalMapPointHasBeenChosen ( i, j ) = true;
      }

    }

  }

}

//------------------------------------------------------------//

inline bool AllochMod::AllochthonousLithologyDistributionGreaterThan::operator ()( const GeoMorphAllochthonousLithologyDistribution* m1,
                       const GeoMorphAllochthonousLithologyDistribution* m2 ) const {
  return m1->getAge () > m2->getAge ();
}

//------------------------------------------------------------//

#endif // __AllochthonousLithologyDistribution_HH__
