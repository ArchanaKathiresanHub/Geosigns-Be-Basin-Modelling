#ifndef _MINING__ELEMENT_POSITION_H_
#define _MINING__ELEMENT_POSITION_H_

#include <string>

#include "Interface/Formation.h"
#include "Interface/Surface.h"

#include "Point.h"

namespace DataAccess {

   namespace Mining {

      /// For a point in 3d space an object of this type contains:
      ///    1. The point in 3d space;
      ///    2. The element in which the point lies ( i, j, k );
      ///    3. The reference co-ordinates of the point in the element;
      ///    4. The formation in which the point lies;
      ///    5. The surface on which the point lies, this may be null.
      ///
      /// The values are only stored in objects of this class, 
      /// they are computed elsewhere.
      class ElementPosition {

      public :

         ElementPosition ();

         ElementPosition ( const ElementPosition& copy );


         /// Assignment operator.
         ElementPosition& operator=( const ElementPosition& rhs );

         void clear ();


         /// Return a const reference to the point in the actual co-ordinate system.
         const Numerics::Point& getActualPoint () const;

         /// Return a reference to the point in the actual co-ordinate system.
         Numerics::Point& getActualPoint ();

         /// Set the actual point.
         void setActualPoint ( const Numerics::Point& newPoint );


         /// Set the depth along hole.
         void setDepthAlongHole ( const double dah );

         /// Get the depth along hole.
         double getDepthAlongHole () const;

         /// Return a const reference to the point in the reference co-ordinate system.
         ///
         /// Based on the element (i,j,k).
         const Numerics::Point& getReferencePoint () const;

         /// Return a const reference to the point in the reference co-ordinate system.
         ///
         /// Based on the element (i,j,k).
         Numerics::Point& getReferencePoint ();

         /// Set the reference point.
         void setReferencePoint ( const Numerics::Point& newPoint );


         /// Get the formation in which the point lies.
         const DataAccess::Interface::Formation* getFormation () const;

         /// Set the formation in which the point lies.
         void setFormation ( const DataAccess::Interface::Formation* formation );


         /// Get the surface on which the point lies.
         ///
         /// This may be null as the point may lie within the formation.
         const DataAccess::Interface::Surface* getSurface () const;

         /// Set the surface on which the point lies.
         void setSurface ( const DataAccess::Interface::Surface* surface );


         /// Get the global I location.
         unsigned int getI () const;

         /// Get the global J location.
         unsigned int getJ () const;

         /// Get the local K location.
         ///
         /// Local from within the associated grid-map.
         unsigned int getLocalK () const;

         /// Get the global K location.
         unsigned int getGlobalK () const;

         bool isValidPlaneElement () const;

         bool isValidElement () const;


         void setMapPosition ( const unsigned int newI,
                               const unsigned int newJ );

         void setDepthPosition ( const unsigned int newGlobalK,
                                 const unsigned int newLocalK );


         void setPosition ( const unsigned int newI,
                            const unsigned int newJ,
                            const unsigned int newGlobalK,
                            const unsigned int newLocalK );

         /// Return a string representation of the element-position.
         std::string image () const;

      private :

         void clearReference ();


         Numerics::Point m_actualPoint;
         Numerics::Point m_referencePoint;
         double          m_depthAlongHole;

         unsigned int m_globalI;
         unsigned int m_globalJ;
         unsigned int m_globalK;
         unsigned int m_localK;

         const DataAccess::Interface::Formation* m_formation;
         const DataAccess::Interface::Surface* m_surface;

      };

      /// \typedef ElementPositionSequence
      /// Order is important here.
      typedef std::vector<ElementPosition> ElementPositionSequence;

   }

}

// Inline functions.

inline const Numerics::Point& DataAccess::Mining::ElementPosition::getActualPoint () const {
   return m_actualPoint;
}

inline Numerics::Point& DataAccess::Mining::ElementPosition::getActualPoint () {
   return m_actualPoint;
}

inline const Numerics::Point& DataAccess::Mining::ElementPosition::getReferencePoint () const {
   return m_referencePoint;
}

inline Numerics::Point& DataAccess::Mining::ElementPosition::getReferencePoint () {
   return m_referencePoint;
}

inline double DataAccess::Mining::ElementPosition::getDepthAlongHole () const {
   return m_depthAlongHole;
}

inline const DataAccess::Interface::Formation* DataAccess::Mining::ElementPosition::getFormation () const {
   return m_formation;
}

inline const DataAccess::Interface::Surface* DataAccess::Mining::ElementPosition::getSurface () const {
   return m_surface;
}

inline unsigned int DataAccess::Mining::ElementPosition::getI () const {
   return m_globalI;
}

inline unsigned int DataAccess::Mining::ElementPosition::getJ () const {
   return m_globalJ;
}

inline unsigned int DataAccess::Mining::ElementPosition::getLocalK () const {
   return m_localK;
}

inline unsigned int DataAccess::Mining::ElementPosition::getGlobalK () const {
   return m_globalK;
}




#endif // _NUMERICS__ELEMENT_POSITION_H_
