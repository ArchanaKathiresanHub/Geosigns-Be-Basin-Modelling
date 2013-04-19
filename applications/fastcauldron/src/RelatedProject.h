#ifndef _FASTCAULDRON_RELATED_PROJECT_H_
#define _FASTCAULDRON_RELATED_PROJECT_H_

#include <string>


#include "Interface/RelatedProject.h"
#include "Interface/Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

// using namespace DataAccess;

class RelatedProject : public DataAccess::Interface::RelatedProject {

public :


   RelatedProject ( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record );

   ~RelatedProject ();

   const std::string& getOutputName () const;

   void setMeshPosition ( const int iPosition,
                          const int jPosition );

   int getMeshPositionI () const;

   int getMeshPositionJ () const;

//    bool onThisProcessor () const;

private :

   int m_iPosition;
   int m_jPosition;
//    bool m_onProcessor;

   std::string m_outputName;

};

#endif // _FASTCAULDRON_RELATED_PROJECT_H_
