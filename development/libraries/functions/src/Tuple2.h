#ifndef _FUNCTIONS_TUPLE2_H_
#define _FUNCTIONS_TUPLE2_H_

#include <assert.h>
#include <limits>
#include <stdexcept>

namespace functions
{
   template <typename ENTRY>
   class Tuple2
   {
   private:

     ENTRY m_first, m_second;

   public:

     Tuple2()
     {
         m_first = -std::numeric_limits<ENTRY>::max();
         m_second = m_first;
     }

     inline ~Tuple2() {}

     explicit Tuple2(const ENTRY* data) {
       if(nullptr==data) throw std::runtime_error("nullptr provided to Tuple2 ctr");
       m_first = data[0];
       m_second = data[1];
     }

     Tuple2(const ENTRY& value1, const ENTRY& value2):
       m_first(value1),
       m_second(value2)
     {}

     inline const ENTRY& operator[](int d) const {
       assert(d == 0 || d == 1);
       return d == 0 ? m_first : m_second;
     }

     inline ENTRY& operator[](int d) {
       assert(d == 0 || d == 1);
       return d == 0 ? m_first : m_second;
     }

     inline Tuple2<ENTRY> reverse() const {
       Tuple2<ENTRY> result;
       result[0] = m_second;
       result[1] = m_first;
       return result;
     }

     inline Tuple2<ENTRY>& operator+=(const Tuple2<ENTRY>& rhs) {
       m_first += rhs[0];
       m_second += rhs[1];
       return *this;
     }

     inline Tuple2<ENTRY>& operator-=(const Tuple2<ENTRY>& rhs) {
       m_first -= rhs[0];
       m_second -= rhs[1];
       return *this;
     }

     inline Tuple2<ENTRY>& operator*=(const Tuple2<ENTRY>& rhs) {
       m_first *= rhs[0];
       m_second *= rhs[1];
       return *this;
     }

     inline Tuple2<ENTRY>& operator/=(const Tuple2<ENTRY>& rhs) {
       m_first /= rhs[0];
       m_second /= rhs[1];
       return *this;
     }
   };

} // namespace functions

#endif
