#ifndef _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_DISTRIBUTION_H_
#define _INETRFACE_ALLOCHTHONOUS_LITHOLOGY_DISTRIBUTION_H_

#include <iostream>
#include <string>
using namespace std;

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {


     class AllochthonousLithologyDistribution : public DAObject
     {
     public:

         AllochthonousLithologyDistribution (ProjectHandle * projectHandle, database::Record * record);

         virtual ~AllochthonousLithologyDistribution (void);

         /// Return the name of the formation of this AllochthonousLithologyDistribution.
         const string& getFormationName (void) const;

         /// Return the age of this AllochthonousLithologyDistribution.
         double getAge (void) const;

         /// Return the the Snapshot of this AllochthonousLithologyDistribution
         const Snapshot * getSnapshot (void) const;

         /// Return the Formation of this AllochthonousLithologyDistribution
         const Formation * getFormation (void) const;

         /// Return the (GridMap) value of one of this AllochthonousLithologyDistribution's attributes
         /// If 0 is returned, no value was specified.
         const GridMap * getMap (const AllochthonousLithologyDistributionMapAttributeId attributeId) const;

         /// load a map
	 GridMap * loadMap ( const AllochthonousLithologyDistributionMapAttributeId attributeId) const;

         /// Print the attributes of this AllochthonousLithologyDistribution
         // May not work if user application is compiled under IRIX with CC -lang:std
         void printOn (ostream & ostr) const;

      protected:

         const Formation* m_formation;
         const Snapshot*  m_snapshot;

         static const string s_MapAttributeNames [];

      };
   }
}

#endif // _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_DISTRIBUTION_H_
