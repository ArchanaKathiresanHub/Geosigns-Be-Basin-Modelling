#ifndef _DERIVED_PROPERTIES__MOCK_SURFACE_H_
#define _DERIVED_PROPERTIES__MOCK_SURFACE_H_

#include <string>

#include "AbstractSurface.h"

namespace DataModel {

   /// \brief 
   class MockSurface : public AbstractSurface {

   public :

      MockSurface ( const std::string& name );

      /// \brief Get the name of the surface.
      virtual const std::string& getName () const;

   private :

      const std::string m_name;

   };

}

inline DataModel::MockSurface::MockSurface ( const std::string& name ) : m_name ( name ) {
}

inline const std::string& DataModel::MockSurface::getName () const {
   return m_name;
}


#endif // _DERIVED_PROPERTIES__MOCK_SURFACE_H_
