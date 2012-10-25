#ifndef _DATAACCESS__VERTICAL_WELL_H_
#define _DATAACCESS__VERTICAL_WELL_H_

#include <string>

#include "CauldronWell.h"
#include "Point.h"

namespace DataAccess {

   namespace Mining {

      class VerticalWell : public CauldronWell {

      public :

         VerticalWell ( const std::string& name );

         ~VerticalWell ();


         void setTop ( const Numerics::Point& top );

         void setLength ( const double length );


         Numerics::Point getTop () const;

         Numerics::Point getLocation ( const double depthAlongHole ) const;

         double getLength () const;

         Numerics::Point getBottom () const;

      private :

         Numerics::Point m_top;
         Numerics::Point m_bottom;
         double          m_length;

      };

   }

}

#endif // _DATAACCESS__VERTICAL_WELL_H_
