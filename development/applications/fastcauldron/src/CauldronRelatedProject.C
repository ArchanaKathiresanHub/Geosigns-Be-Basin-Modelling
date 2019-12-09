#include "CauldronRelatedProject.h"

#include "utils.h"


CauldronRelatedProject::CauldronRelatedProject (DataAccess::Interface::ProjectHandle& projectHandle, database::Record * record ) : DataAccess::Interface::RelatedProject ( projectHandle, record ) {

   if ( getName ().find ( "*Point" ) != std::string::npos ) {

      m_outputName = "Point_"
         + IntegerToString ( int ( getEast ())) + "_east__"
         + IntegerToString ( int ( getNorth ())) + "_north";

   } else {
      std::string::size_type pos = getName ().find (".",0);

      m_outputName = getName ().substr( 0, pos );
   }

}

CauldronRelatedProject::~CauldronRelatedProject () {
}

const std::string& CauldronRelatedProject::getOutputName () const {
   return m_outputName;
}

void CauldronRelatedProject::setMeshPosition ( const int iPosition,
                                       const int jPosition ) {
   m_iPosition = iPosition;
   m_jPosition = jPosition;
}

int CauldronRelatedProject::getMeshPositionI () const {
   return m_iPosition;
}

int CauldronRelatedProject::getMeshPositionJ () const {
   return m_jPosition;
}
