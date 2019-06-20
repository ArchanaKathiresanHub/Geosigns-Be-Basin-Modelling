#ifndef _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_H_
#define _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_H_

#include <iostream>
using namespace std;

#include <string>
using namespace std;

#include "Interface.h"
#include "DAObject.h"

namespace DataAccess
{
   namespace Interface
   {

      class Formation;
      class LithoType;
      class AllochthonousLithologyDistribution;
      class AllochthonousLithologyInterpolation;

     class AllochthonousLithology : public DAObject {
     public:

       AllochthonousLithology (ProjectHandle * projectHandle, database::Record * record);
       ~AllochthonousLithology (void);


       /// Return the layer name of this AllochthonousLithology
       const string& getFormationName (void) const;

       /// Return the name of the lithology of this AllochthonousLithology
       const string& getLithologyName (void) const;

       const Formation * getFormation (void) const;

       const LithoType * getLithoType (void) const;

       /// return the list of AllochthonousLithologies in this formation.
       AllochthonousLithologyDistributionList * getAllochthonousLithologyDistributions (void) const;

       /// return the list of AllochthonousLithologies in this formation.
       AllochthonousLithologyInterpolationList * getAllochthonousLithologyInterpolations (void) const;

       /// Print the attributes of this AllochthonousLithology
       // May not work if user application is compiled under IRIX with CC -lang:std
       void printOn (ostream &) const;
       
     protected:

       mutable const Formation* m_formation;
       mutable const LithoType* m_lithoType;

     };

   }

}

#endif // _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_H_
