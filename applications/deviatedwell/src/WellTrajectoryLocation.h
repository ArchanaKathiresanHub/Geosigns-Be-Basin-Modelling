#ifndef __DEVIATED_WELL__WELL_TRAJECTORY_LOCATION__H_
#define __DEVIATED_WELL__WELL_TRAJECTORY_LOCATION__H_

#include "Point.h"

/// Has the position of input point.
///
/// The depth (z) may be a null value, if so then this indicates 
/// the position in the plane of the well location.
/// The depth-along-hole may be a null value.
class WellTrajectoryLocation {

public :

   /// Simple constructor.
   WellTrajectoryLocation ();

   /// Constructor.
   ///
   /// depth and depth-along-hole are set to null-value.
   WellTrajectoryLocation ( const double x,
                            const double y );

   /// Constructor.
   ///
   /// depth-along-hole are set to null-value.
   WellTrajectoryLocation ( const double x,
                            const double y,
                            const double z );

   /// Constructor.
   WellTrajectoryLocation ( const double x,
                            const double y,
                            const double z,
                            const double s );


   /// Set the position of the location.
   ///
   /// depth and depth-along-hole are set to null-value.
   void set ( const double x,
              const double y );

   /// Set the position of the location.
   ///
   /// depth-along-hole are set to null-value.
   void set ( const double x,
              const double y,
              const double z );

   /// Set the position of the location.
   void set ( const double x,
              const double y,
              const double z,
              const double s );

   /// Get the position in space of the point.
   const Numerics::Point& position () const;

   /// Get distance of the point along the well-path.
   double depthAlongHole () const;

private :

   /// The position in space of the point.
   Numerics::Point  m_position;

   /// The distance of the point along the well-path.
   double m_depthAlongHole;

};


// Inline functions.

inline const Numerics::Point& WellTrajectoryLocation::position () const {
   return m_position;
}

inline double WellTrajectoryLocation::depthAlongHole () const {
   return m_depthAlongHole;
}


#endif // __DEVIATED_WELL__WELL_TRAJECTORY_LOCATION__H_
