#ifndef _INTERFACE_RELATEDPROJECT_H_
#define _INTERFACE_RELATEDPROJECT_H_


#include "DAObject.h"
#include "Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// A related project a a 1-d project associated with a particular location in the 3-d project.
      class RelatedProject : public DAObject
      {
      public:

         RelatedProject (ProjectHandle& projectHandle, database::Record * record);

         virtual ~RelatedProject ();

         /// Returns the northern location of the related-project.
         virtual double getNorth () const;

         /// Returns the eastern location of the related-project.
         virtual double getEast () const;

	 // returns top of sediment of related project
	 virtual double getTopOfSediment () const;

	 // returns bottom of sediment of related project
	 virtual double getBottomOfSediment () const;

         /// Return the name of the related project.
         ///
         /// If this is a 1-d project then the project name will be returned,
         /// otherwise it will be a string resembling "*Point XXX east, YYY north".
         /// Where XXX and YYY are the coordinates.
         virtual const std::string& getName () const;

         /// Print the attributes of this MobileLayer
         // May not work if user application is compiled under IRIX with CC -lang:std
         virtual void printOn (ostream & ostr) const;

         virtual void asString (std::string & str) const;

      protected :

         std::string m_projectName;

         double m_eastPosition;
         double m_northPosition;

	 double m_topOfSediment;
	 double m_bottomOfSediment;

			};

	 }
}
#endif // _INTERFACE_RELATEDPROJECT_H_
