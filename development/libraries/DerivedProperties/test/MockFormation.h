//
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DERIVED_PROPERTIES__MOCK_FORMATION_H_
#define _DERIVED_PROPERTIES__MOCK_FORMATION_H_

#include <string>
#include <iostream>

#include "AbstractFormation.h"
#include "MockCompoundLithologyArray.h"

namespace DataModel {

   /// \brief A formation.
   class MockFormation final: public AbstractFormation {

   public :

      MockFormation ( const std::string& name,
                      const std::string& topSurfaceName = "",
                      const std::string& bottomSurfaceName = "",
                      const unsigned int numberMaxElements = 0  );

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

      virtual void printOn ( std::ostream& os ) const;

      /// \brief Get the Compound lithology array.
      virtual const DataModel::MockCompoundLithologyArray& getCompoundLithologyArray () const final;

      /// \brief Output information about the formation to the ostream.
      virtual unsigned int getMaximumNumberOfElements () const final;

   private :

      const std::string m_name;
      const std::string m_topSurfaceName;
      const std::string m_bottomSurfaceName;
      const unsigned int m_numberMaxElements;
      const MockCompoundLithologyArray m_mockCompoundLithologyArray;


   };

} // namespace DataModel

inline DataModel::MockFormation::MockFormation ( const std::string& name,
                                                 const std::string& topSurfaceName,
                                                 const std::string& bottomSurfaceName,
                                                 const unsigned int numberMaxElements ) :
   m_name ( name ),
   m_topSurfaceName ( topSurfaceName ),
   m_bottomSurfaceName ( bottomSurfaceName ),
   m_numberMaxElements ( numberMaxElements ),
   m_mockCompoundLithologyArray()
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

inline void DataModel::MockFormation::printOn ( std::ostream& os ) const {
   os << "MockFormation::" << getName () << std::endl;
}

inline unsigned int DataModel::MockFormation::getMaximumNumberOfElements () const {
   return m_numberMaxElements;
}

inline const DataModel::MockCompoundLithologyArray& DataModel::MockFormation::getCompoundLithologyArray () const {
   return m_mockCompoundLithologyArray;
}

#endif // _DERIVED_PROPERTIES__MOCK_FORMATION_H_
