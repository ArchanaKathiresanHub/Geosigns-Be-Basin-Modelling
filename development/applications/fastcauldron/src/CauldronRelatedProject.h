#ifndef _FASTCAULDRON_RELATED_PROJECT_H_
#define _FASTCAULDRON_RELATED_PROJECT_H_

#include <string>


#include "RelatedProject.h"
#include "Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}

// using namespace DataAccess;

class CauldronRelatedProject : public DataAccess::Interface::RelatedProject {

public :


   CauldronRelatedProject ( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record );

   ~CauldronRelatedProject ();

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
