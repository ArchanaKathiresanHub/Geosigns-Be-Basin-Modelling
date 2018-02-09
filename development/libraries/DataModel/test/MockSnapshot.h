//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__MOCK_SNAPSHOT_H
#define DERIVED_PROPERTIES__MOCK_SNAPSHOT_H

#include "AbstractSnapshot.h"

namespace DataModel {

   /// \brief The mock snapshot used for unit testing
   class MockSnapshot : public AbstractSnapshot {

   public :

      MockSnapshot ( const double age ) : m_age ( age ) {}

      /// \brief Get the age at which the snapshot is defined.
      double getTime () const final;

   private :

      const double m_age;

   };

} // namespace DataModel

inline double DataModel::MockSnapshot::getTime () const {
   return m_age;
}

#endif // DERIVED_PROPERTIES__MOCK_SNAPSHOT_H
