#ifndef _DERIVED_PROPERTIES__MOCK_FORMATION_H_
#define _DERIVED_PROPERTIES__MOCK_FORMATION_H_

#include <string>

#include "AbstractFormation.h"

namespace DataModel {

   /// \brief A formation.
   class MockFormation : public AbstractFormation {

   public :

      MockFormation ( const std::string& name,
                      const std::string& topSurfaceName = "",
                      const std::string& bottomSurfaceName = ""  );

      /// \brief Get the name of the formation.
      virtual const std::string& getName () const;

      /// \brief Return the name of the Surface at the top of this Formation.
      ///
      /// If there is no surface above then a null string ("") will be returned.
      virtual const std::string& getTopSurfaceName () const;

      /// \brief Return the name of the Surface at the bottom of this Formation.
      ///
      /// If there is no surface below then a null string ("") will be returned.
      virtual const std::string& getBottomSurfaceName () const;

   private :

      const std::string m_name;
      const std::string m_topSurfaceName;
      const std::string m_bottomSurfaceName;

   };

} // namespace DataModel

inline DataModel::MockFormation::MockFormation ( const std::string& name,
                                                 const std::string& topSurfaceName,
                                                 const std::string& bottomSurfaceName ) :
   m_name ( name ),
   m_topSurfaceName ( topSurfaceName ),
   m_bottomSurfaceName ( bottomSurfaceName )
{
}

inline const std::string& DataModel::MockFormation::getName () const {
   return m_name;
}

inline const std::string& DataModel::MockFormation::getTopSurfaceName () const {
   return m_topSurfaceName;
}

inline const std::string& DataModel::MockFormation::getBottomSurfaceName () const {
   return m_bottomSurfaceName;
}


#endif // _DERIVED_PROPERTIES__MOCK_FORMATION_H_
