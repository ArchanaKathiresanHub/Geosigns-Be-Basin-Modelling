#include "RelatedProject.h"

#include "utils.h"


RelatedProject::RelatedProject ( DataAccess::Interface::ProjectHandle * projectHandle, database::Record * record ) : DataAccess::Interface::RelatedProject ( projectHandle, record ) {

   if ( getName ().find ( "*Point" ) != std::string::npos ) {

      m_outputName = "Point_"
         + IntegerToString ( int ( getEast ())) + "_east__"
         + IntegerToString ( int ( getNorth ())) + "_north";

   } else {
      std::string::size_type pos = getName ().find (".",0);

      m_outputName = getName ().substr( 0, pos );
   }

}

RelatedProject::~RelatedProject () {
}

const std::string& RelatedProject::getOutputName () const {
   return m_outputName;
}

void RelatedProject::setMeshPosition ( const int iPosition,
                                       const int jPosition ) {
   m_iPosition = iPosition;
   m_jPosition = jPosition;
}

int RelatedProject::getMeshPositionI () const {
   return m_iPosition;
}

int RelatedProject::getMeshPositionJ () const {
   return m_jPosition;
}
