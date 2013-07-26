#ifndef _DATA_ACCESS__PERMAFROST_EVENT__H_
#define _DATA_ACCESS__PERMAFROST_EVENT__H_

using namespace std;

#include <string>

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{

   namespace Interface
   {

      /// \brief A PermafrostEvent object contains information about a permafrost modelling.
      class PermafrostEvent : public DAObject
      {
      public:

         PermafrostEvent ( ProjectHandle* projectHandle, database::Record * record );

         virtual ~PermafrostEvent ();

         /// \brief Indicate whether permafrost is being modelled
         virtual bool getPermafrost () const;

         /// \brief Indicate whether salinity term is being used in permafrost modelling
         virtual bool getSalinityTerm () const;

         /// \brief Indicate whether pressure term is being used in permafrost modelling
         virtual bool getPressureTerm () const;

         ///\brief Activate/deactivate permafrost modelling
         virtual void setPermafrost( const bool aPermafrost ) ;

         /// \brief Return the strnig representation of the permafrot event object.
         virtual std::string image () const;

      private :
         bool m_permafrost;
         bool m_pressureTerm;
         bool m_salinityTerm;

      };

   }

}


#endif // _DATA_ACCESS__PERMAFROST_EVENT__H_
