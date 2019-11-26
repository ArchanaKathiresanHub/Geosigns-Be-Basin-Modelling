#include "PointAdsorptionHistory.h"

#include <iomanip>
#include <sstream>

#include "mangle.h"

#include "cauldronschemafuncs.h"
#include "database.h"

DataAccess::Interface::PointAdsorptionHistory::PointAdsorptionHistory (  ProjectHandle*    projectHandle,
                                                                         database::Record* record ) :
  DAObject ( projectHandle, record ),
  m_x{0.0},
  m_y{0.0},
  m_formationName{""},
  m_mangledFormationName{""},
  m_fileName{""}
{
  if (record != nullptr)
  {
    setFromRecord(record);
  }
}

DataAccess::Interface::PointAdsorptionHistory::~PointAdsorptionHistory () {
}

void DataAccess::Interface::PointAdsorptionHistory::setFromRecord ( database::Record* record )
{
  m_x = database::getXPlanePosition ( record );
  m_y = database::getYPlanePosition ( record );
  //   m_formationName = database::getAdsorptionFormationName ( record );
  m_formationName = database::getFormationName ( record );
  m_mangledFormationName = utilities::mangle ( m_formationName );
  m_fileName = database::getFilename ( record );
}

double DataAccess::Interface::PointAdsorptionHistory::getX () const {
  return m_x;
}

double DataAccess::Interface::PointAdsorptionHistory::getY () const {
  return m_y;
}

const std::string& DataAccess::Interface::PointAdsorptionHistory::getFileName () const {
  return m_fileName;
} 

const std::string& DataAccess::Interface::PointAdsorptionHistory::getFormationName () const {
  return m_formationName;
}

const std::string& DataAccess::Interface::PointAdsorptionHistory::getMangledFormationName () const {
  return m_mangledFormationName;
}


std::string DataAccess::Interface::PointAdsorptionHistory::image () const {

  std::stringstream buffer;

  buffer << " history-point: " << getFormationName ()
         << std::setw ( 15 ) << getX  ()
         << std::setw ( 15 ) << getY  ()
         << std::endl;

  return buffer.str ();
}
