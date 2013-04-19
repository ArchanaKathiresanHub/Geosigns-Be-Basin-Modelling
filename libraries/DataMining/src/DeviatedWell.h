#ifndef _DATAACCESS__DEVIATED_WELL_H_
#define _DATAACCESS__DEVIATED_WELL_H_

#include <string>

#include "CauldronWell.h"
#include "Point.h"
#include "PieceWiseInterpolator1D.h"

namespace DataAccess {

   namespace Mining {

      class DeviatedWell : public CauldronWell {

      public :

         DeviatedWell ( const std::string& name );

         ~DeviatedWell ();

         /// Add a point to the deviated well.
         ///
         /// (x,y,z) is the point in space, s is the depth-along-hole.
         void addLocation ( const double x,
                            const double y,
                            const double z,
                            const double s );

         void addLocation ( const Numerics::Point& p,
                            const double           s );

         /// No more points can be added after this is called.
         void freeze ( const Numerics::PieceWiseInterpolator1D::InterpolationKind kind = Numerics::PieceWiseInterpolator1D::PIECEWISE_LINEAR,
                       const bool                                       allowExtrapolation = false );

         Numerics::Point getTop () const;

         Numerics::Point getLocation ( const double depthAlongHole ) const;

         double getLength () const;

         Numerics::Point getBottom () const;

      private :

         Numerics::PieceWiseInterpolator1D m_x;
         Numerics::PieceWiseInterpolator1D m_y;
         Numerics::PieceWiseInterpolator1D m_z;
         double m_length;

      };

   }

}

#endif // _DATAACCESS__DEVIATED_WELL_H_
