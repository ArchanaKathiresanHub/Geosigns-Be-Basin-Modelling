#ifndef _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_
#define _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_

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
            return m_bioConsts[ index ] * m_tempFactor;
         }
      };
   } // namespace DataAccess::Interface
} // namespace DataAccess

#endif
