#ifndef _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_
#define _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_

#include <iostream>
#include <vector>
#include <assert.h>

namespace DataAccess {

   namespace Interface {

      class BioConsts
      {
      private:

         double m_tempFactor;
         std::vector<double> m_bioConsts;

      public:

         BioConsts( const double& tempFactor, const vector<double>& bioConsts ) :
            m_tempFactor( tempFactor ),
            m_bioConsts( bioConsts )
         {}

         size_t size() const { return m_bioConsts.size(); }

         double operator[]( int index ) const
         {
            assert( index >= 0 && index < (int)m_bioConsts.size() );
            if (m_bioConsts[index] < 0.0)
            {
               cerr << "Warning: the biodegradation coefficient of the component " << index << " is negative: " << m_bioConsts[index] << ". No biodegradation computed for this component" << endl;
               return -199999;
            }
            if (m_tempFactor < 0.0)
            {
               cerr << "Warning: The temperature factor coefficient used for biodegradation is negative: " << m_tempFactor << ". No biodegradation computed" << endl;
               return -199999;
            }
            return m_bioConsts[ index ] * m_tempFactor;
         }
      };
   } // namespace DataAccess::Interface
} // namespace DataAccess

#endif
