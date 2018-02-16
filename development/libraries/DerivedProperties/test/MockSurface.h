// 
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef _DERIVED_PROPERTIES__MOCK_SURFACE_H_
#define _DERIVED_PROPERTIES__MOCK_SURFACE_H_

#include <string>

#include "AbstractSurface.h"
#include "AbstractFormation.h"

namespace DataModel {

   /// \brief 
   class MockSurface final: public AbstractSurface {

   public :

      MockSurface ( const std::string& name,
                    const AbstractFormation* topFormation = 0,
                    const AbstractFormation* bottomFormation = 0 );

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

      virtual const AbstractFormation* getTopFormation () const;

      virtual const AbstractFormation* getBottomFormation () const;

   private :

      const std::string m_name;
      const std::string m_topFormationName;
      const std::string m_bottomFormationName;

      const AbstractFormation* m_topFormation;
      const AbstractFormation* m_bottomFormation;

   };

} // namespace DataModel

inline DataModel::MockSurface::MockSurface ( const std::string& name,
                                             const DataModel::AbstractFormation* topFormation,
                                             const DataModel::AbstractFormation* bottomFormation ) :
   m_name ( name ),
   m_topFormationName ( topFormation == 0 ? "" : topFormation->getName ()),
   m_bottomFormationName ( bottomFormation == 0 ? "" : bottomFormation->getName ()),
   m_topFormation ( topFormation ),
   m_bottomFormation ( bottomFormation )
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

inline const DataModel::AbstractFormation* DataModel::MockSurface::getTopFormation () const {
   return m_topFormation;
}

inline const DataModel::AbstractFormation* DataModel::MockSurface::getBottomFormation () const {
   return m_bottomFormation;
}


#endif // _DERIVED_PROPERTIES__MOCK_SURFACE_H_
