#include "SourceRockAdsorptionHistory.h"
#include <FilePath.h>

#include <fstream>
#include <sstream>

Genex6::SourceRockAdsorptionHistory::SourceRockAdsorptionHistory (DataAccess::Interface::ProjectHandle& projectHandle,
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

void Genex6::SourceRockAdsorptionHistory::save ()
{
   if (m_historyRecord != 0 and (m_adsorptionHistory != 0 || m_genexHistory != 0))
   {
      std::string fileName;

      if (m_historyRecord->getFileName () == "")
      {
         std::stringstream buffer;

         buffer << "History_"
                << m_projectHandle.getProjectName ()
                << "_"
                << (m_adsorptionHistory != 0 ? "shalegas" : "genex")
                << "_" << m_historyRecord->getMangledFormationName ();

         if (m_projectHandle.getModellingMode () == DataAccess::Interface::MODE3D)
         {
            buffer << "_"
                   << m_historyRecord->getX ()
                   << "_"
                   << m_historyRecord->getY ();
         }
         buffer << ".dat";

         fileName = buffer.str ();
      }
      else
      {
         fileName = m_historyRecord->getFileName ();
      }


      if (!m_projectHandle.makeOutputDir ())
         return;

      ibs::FilePath filePath( m_projectHandle.getOutputDir () );
      filePath << fileName;

      std::ofstream historyFile (filePath.cpath (), std::ios::out);

      if( m_adsorptionHistory != 0 ) {
         historyFile << m_historyRecord->getFormationName ();

         if (m_projectHandle.getModellingMode () == DataAccess::Interface::MODE3D )
         {
            historyFile << "  "
                        << m_historyRecord->getX ()
                        << "  " << m_historyRecord->getY ()
                        << std::endl;
         }
         m_adsorptionHistory->write (historyFile);
      } else {
         m_genexHistory->write ( historyFile );
      }
      historyFile.close ();
   }
}
