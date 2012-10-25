#ifndef _INTERFACE_LEAD_H_
#define _INTERFACE_LEAD_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// Lead Class
      class Lead : public DAObject
      {
	 public:
	    Lead (ProjectHandle * projectHandle, unsigned int id);
	    ~Lead (void);

	    /// Return the id of this Trap
	    virtual unsigned int getId (void) const;

	    /// Return the list of Lead Traps that belong to this Lead
	    virtual LeadTrapList * getLeadTraps (void) const;

	    LeadTrap * createLeadTrap (Trap * trap);

	    LeadTrap * findLeadTrap (Reservoir * reservoir, unsigned int id) const;

	    virtual void printOn (ostream &) const;
	    virtual void asString (string &) const;

	 private:
	    /// mutable list type for the LeadTraps
	    typedef vector<LeadTrap *> MutableLeadTrapList;

	    const unsigned int m_id;
	    MutableLeadTrapList m_leadTraps;

      };
   }
}

#endif // _INTERFACE_LEAD_H_
