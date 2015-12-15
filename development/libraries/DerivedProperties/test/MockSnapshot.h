#ifndef _DERIVED_PROPERTIES__MOCK_SNAPSHOT_H_
#define _DERIVED_PROPERTIES__MOCK_SNAPSHOT_H_

#include "AbstractSnapshot.h"

namespace DataModel {

   /// \brief The age at which.
   class MockSnapshot : public AbstractSnapshot {

   public :

      MockSnapshot ( const double age );

      /// \brief Get the age at which the snapshot is defined.
      virtual double getTime () const;

   private :

      double m_age;

   };

} // namespace DataModel

inline DataModel::MockSnapshot::MockSnapshot ( const double age ) : m_age ( age ) {
}

inline double DataModel::MockSnapshot::getTime () const {
   return m_age;
}


#endif // _DERIVED_PROPERTIES__MOCK_SNAPSHOT_H_
