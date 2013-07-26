#include "SourceRockAdsorptionHistory.h"
#include <fstream>
#include <sstream>

SourceRockAdsorptionHistory::SourceRockAdsorptionHistory ( Interface::ProjectHandle* projectHandle, 
                                                           AdsorptionProjectHandle*       adsorptionProjectHandle,
                                                           PointAdsorptionHistory*        record ) :
   m_projectHandle ( projectHandle ),
   m_adsorptionProjectHandle ( adsorptionProjectHandle ),
   m_historyRecord ( record ),
   m_adsorptionHistory ( 0 )
{
}


void SourceRockAdsorptionHistory::setNodeAdsorptionHistory ( Genex5::NodeAdsorptionHistory* adsorptionHistory ) {
   m_adsorptionHistory = adsorptionHistory;
}


Genex5::NodeAdsorptionHistory* SourceRockAdsorptionHistory::getNodeAdsorptionHistory () {
   return m_adsorptionHistory;
}

void SourceRockAdsorptionHistory::save () {

   if ( m_historyRecord != 0 and m_adsorptionHistory != 0 ) {
      std::stringstream buffer;

      buffer << m_projectHandle->getFullOutputDir () <<  "/History_" << m_projectHandle->getProjectName () << "_" << m_historyRecord->getMangledFormationName () << "_" << m_historyRecord->getX () << "_" << m_historyRecord->getY () << ".dat";

      std::ofstream historyFile ( buffer.str ().c_str (), std::ios::out );

      historyFile << m_historyRecord->getFormationName () << "  " << m_historyRecord->getX () << "  " << m_historyRecord->getY () << std::endl;

      m_adsorptionHistory->write ( historyFile );
      historyFile.close ();
   }

}

