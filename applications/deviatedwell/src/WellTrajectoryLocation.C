#include "WellTrajectoryLocation.h"

#include "Interface/Interface.h"

WellTrajectoryLocation::WellTrajectoryLocation () {
}

WellTrajectoryLocation::WellTrajectoryLocation ( const double x,
                                                 const double y ) {
   set ( x, y );
}

WellTrajectoryLocation::WellTrajectoryLocation ( const double x,
                                                 const double y,
                                                 const double z ) {
   set ( x, y, z );
}

WellTrajectoryLocation::WellTrajectoryLocation ( const double x,
                                                 const double y,
                                                 const double z,
                                                 const double s ) {
   set ( x, y, z, s );
}

void WellTrajectoryLocation::set ( const double x,
                                   const double y ) {
   m_position ( 0 ) = x;
   m_position ( 1 ) = y;
   m_position ( 2 ) = DataAccess::Interface::DefaultUndefinedMapValue;

   m_depthAlongHole = DataAccess::Interface::DefaultUndefinedMapValue;
}

void WellTrajectoryLocation::set ( const double x,
                                   const double y,
                                   const double z ) {
   m_position ( 0 ) = x;
   m_position ( 1 ) = y;
   m_position ( 2 ) = z;

   m_depthAlongHole = DataAccess::Interface::DefaultUndefinedMapValue;
}

void WellTrajectoryLocation::set ( const double x,
                                   const double y,
                                   const double z,
                                   const double s ) {

   m_position ( 0 ) = x;
   m_position ( 1 ) = y;
   m_position ( 2 ) = z;

   m_depthAlongHole = s;
}
