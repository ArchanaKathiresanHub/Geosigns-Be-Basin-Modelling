#ifndef _DERIVED_PROPERTIES__MOCK_SURFACE_H_
#define _DERIVED_PROPERTIES__MOCK_SURFACE_H_

#include <string>

#include "AbstractSurface.h"

namespace DataModel {

   /// \brief 
   class MockSurface : public AbstractSurface {

   public :

      MockSurface ( const std::string& name,
                    const std::string& topFormationName = "",
                    const std::string& bottomFormationName = "" );

      /// \brief Get the name of the surface.
      virtual const std::string& getName () const;

      /// \brief Return the name of the Formation found above this Surface if there is one.
      ///
      /// If there is no formation above then a null string ("") will be returned.
      virtual const std::string& getTopFormationName () const;

      /// \brief Return the name of the Formation found below this Surface if there is one.
      ///
      /// If there is no formation below then a null string ("") will be returned.
      virtual const std::string& getBottomFormationName () const;

   private :

      const std::string m_name;
      const std::string m_topFormationName;
      const std::string m_bottomFormationName;

   };

}

inline DataModel::MockSurface::MockSurface ( const std::string& name,
                                             const std::string& topFormationName,
                                             const std::string& bottomFormationName ) :
   m_name ( name ),
   m_topFormationName ( topFormationName ),
   m_bottomFormationName ( bottomFormationName )
{
}

inline const std::string& DataModel::MockSurface::getName () const {
   return m_name;
}

inline const std::string& DataModel::MockSurface::getTopFormationName () const {
   return m_topFormationName;
}

inline const std::string& DataModel::MockSurface::getBottomFormationName () const {
   return m_bottomFormationName;
}


#endif // _DERIVED_PROPERTIES__MOCK_SURFACE_H_
