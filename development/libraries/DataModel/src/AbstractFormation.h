//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _DATA_MODEL__ABSTRACT_FORMATION_H_
#define _DATA_MODEL__ABSTRACT_FORMATION_H_

#include <string>
#include "AbstractCompoundLithologyArray.h"

namespace DataModel {

   /// \brief A formation.
   class AbstractFormation {

   public :

      virtual ~AbstractFormation() = default;

      /// \brief Get the name of the formation.
      virtual const std::string& getName () const = 0;

      /// \brief Return the name of the Surface at the top of this Formation.
      ///
      /// If there is no surface above then a null string ("") will be returned.
      virtual const std::string& getTopSurfaceName () const = 0;

      /// \brief Return the name of the Surface at the bottom of this Formation.
      ///
      /// If there is no surface below then a null string ("") will be returned.
      virtual const std::string& getBottomSurfaceName () const = 0;

      /// \brief Output information about the formation to the ostream.
      virtual void printOn ( std::ostream& os ) const = 0;
      
      /// \brief Get the Compound lithology array.
      virtual const AbstractCompoundLithologyArray& getCompoundLithologyArray () const = 0;
      
      /// \brief Output the maximum of elements number
      virtual unsigned int getMaximumNumberOfElements () const = 0;

   };

} // namespace DataModel

#endif // _DATA_MODEL__ABSTRACT_FORMATION_H_

