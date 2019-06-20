#ifndef _INTERFACE_PROJECT_DATA_H_
#define _INTERFACE_PROJECT_DATA_H_


#include "DAObject.h"
#include "Interface.h"

#include <string>

namespace DataAccess
{
   namespace Interface
   {

      /// Contains some general information about a project, its name, size and any sub-sampling parameters.
      class ProjectData : public DAObject
      {
      public :

         ProjectData ( ProjectHandle * projectHandle, database::Record* record );

         virtual ~ProjectData ();

         /// Return name of project.
         virtual const std::string& getProjectName () const;

         /// Return description of project.
         virtual const std::string& getDescription () const;

         /// Return the cauldron version that created the project.
         virtual const std::string& getProgramVersion () const;

 
         /// Return x-origin of the map.
         virtual double getXOrigin () const;

         /// Return y-origin of the map.
         virtual double getYOrigin () const;

         /// Return delta-x in the map.
         virtual double getDeltaX () const;

         /// Return delta-y in the map.
         virtual double getDeltaY () const;


         /// Return the number of node in the x-direction.
         virtual int getNumberOfXNodes () const;

         /// Return the number of node in the y-direction.
         virtual int getNumberOfYNodes () const;

         /// Return the sub-sampling inthe x-direction.
         virtual int getXNodeStep () const;

         /// Return the sub-sampling inthe y-direction.
         virtual int getYNodeStep () const;

         /// Return the number of nodes from the start position for the model area.
         virtual int getXNodeOffset () const;

         /// Return the number of nodes from the start position for the model area.
         virtual int getYNodeOffset () const;

         /// Return the x-start position for the model area.
         virtual int getWindowXMin () const;

         /// Return the x-end position for the model area.
         virtual int getWindowXMax () const;

         /// Return the y-start position for the model area.
         virtual int getWindowYMin () const;

         /// Return the y-end position for the model area.
         virtual int getWindowYMax () const;


      };

   }

}

#endif // _INTERFACE_PROJECT_DATA_H_
