#ifndef _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_
#define _DISTRIBUTEDDATAACCESS_BIOCONSTS_H_

#include <vector>
#include <assert.h>

namespace DataAccess { namespace Interface {

using namespace std;

class BioConsts
{
private:

   double m_tempFactor;
   vector<double> m_bioConsts;

public:

   BioConsts(const double& tempFactor, const vector<double>& bioConsts):
     m_tempFactor(tempFactor),
     m_bioConsts(bioConsts)
   {}

   int size() const { return m_bioConsts.size(); }

   double operator[](int index) const 
   {
     assert(index >= 0 && index < (int)m_bioConsts.size());
     return m_bioConsts[index] * m_tempFactor;
   }
};

} } // namespace DataAccess::Interface

#endif
