#ifndef _INTERFACE_FAULTCOLLECTION_H_
#define _INTERFACE_FAULTCOLLECTION_H_

#include <string>
#include <vector>
#include <map>

using namespace std;

#include "auxiliaryfaulttypes.h"
#include "DAObject.h"
#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      class FaultCollection : public DAObject
      {
      public:
         ///
         /// A mapping from the fault name to the fault.
         ///
         typedef std::map < std::string, Fault * >NameFaultMapping;

         /// constructor
         FaultCollection (ProjectHandle& projectHandle, const string & mapName);

         /// destructor
         virtual ~FaultCollection (void);

   virtual const string & getName (void) const;

         void addFormation (const Formation * formation);

         virtual bool appliesToFormation (const Formation * formation) const;

         void addFault (const std::string & faultName, const PointSequence & faultLine);

         void addEvent (const std::string & faultName, const Snapshot * snapshot, const string & status);

         void addOverpressureEvent (const std::string & faultName,
                                    const Snapshot * snapshot,
                                    const string & faultLithology,
                                    const bool usedInOverpressure );

   Fault * findFault (const std::string & faultName) const;

   FaultList * getFaults (void) const;

         virtual size_t size (void) const;

      private:
         const string m_name;
         NameFaultMapping m_faults;
         FormationList m_formations;

         friend ostream & operator<< (ostream & o, const FaultCollection & collection);
      };
   }
}
#endif // _INTERFACE_FAULTCOLLECTION_H_

