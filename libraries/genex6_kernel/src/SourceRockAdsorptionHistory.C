#include "SourceRockAdsorptionHistory.h"
#include <fstream>
#include <sstream>

Genex6::SourceRockAdsorptionHistory::SourceRockAdsorptionHistory ( DataAccess::Interface::ProjectHandle*                projectHandle, 
                                                                   const DataAccess::Interface::PointAdsorptionHistory* record ) :
   m_projectHandle ( projectHandle ),
   m_historyRecord ( record ),
   m_adsorptionHistory ( 0 ),
   m_genexHistory ( 0 )
{
}


void Genex6::SourceRockAdsorptionHistory::setNodeAdsorptionHistory ( Genex6::NodeAdsorptionHistory* adsorptionHistory ) {
   m_adsorptionHistory = adsorptionHistory;
}


void Genex6::SourceRockAdsorptionHistory::setNodeGenexHistory ( Genex6::NodeAdsorptionHistory* genexHistory ) {
   m_genexHistory = genexHistory;
}


Genex6::NodeAdsorptionHistory* Genex6::SourceRockAdsorptionHistory::getNodeAdsorptionHistory () {
   return m_adsorptionHistory;
}

Genex6::NodeAdsorptionHistory* Genex6::SourceRockAdsorptionHistory::getNodeGenexHistory () {
   return m_genexHistory;
}

void Genex6::SourceRockAdsorptionHistory::save () {

   if ( m_historyRecord != 0 and m_adsorptionHistory != 0 ) {
      std::stringstream buffer;

      buffer << m_projectHandle->getOutputDir () 
             <<  "/History_"
             << m_projectHandle->getProjectName ()
             << "_"
             << "shalegas"
             << "_"
             << m_historyRecord->getMangledFormationName ()
             << "_"
             << m_historyRecord->getX ()
             << "_"
             << m_historyRecord->getY ()
             << ".dat";

      std::ofstream historyFile ( buffer.str ().c_str (), std::ios::out );

      historyFile << m_historyRecord->getFormationName ()
                  << "  "
                  << m_historyRecord->getX ()
                  << "  "
                  << m_historyRecord->getY ()
                  << std::endl;

      m_adsorptionHistory->write ( historyFile );
      historyFile.close ();
   }

   if ( m_historyRecord != 0 and m_genexHistory != 0 ) {
      std::stringstream buffer;

      buffer << m_projectHandle->getOutputDir () 
             <<  "/History_"
             << m_projectHandle->getProjectName ()
             << "_"
             << "genex"
             << "_"
             << m_historyRecord->getMangledFormationName ()
             << "_"
             << m_historyRecord->getX ()
             << "_"
             << m_historyRecord->getY ()
             << ".dat";

      std::ofstream historyFile ( buffer.str ().c_str (), std::ios::out );

      // historyFile << m_historyRecord->getFormationName ()
      //             << "  "
      //             << m_historyRecord->getX ()
      //             << "  "
      //             << m_historyRecord->getY ()
      //             << std::endl;

      m_genexHistory->write ( historyFile );
      historyFile.close ();
   }

}

