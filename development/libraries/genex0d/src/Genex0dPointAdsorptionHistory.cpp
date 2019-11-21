#include "Genex0dPointAdsorptionHistory.h"

#include "Genex0dInputData.h"

#include "mangle.h"

namespace genex0d
{

Genex0dPointAdsorptionHistory::Genex0dPointAdsorptionHistory(DataAccess::Interface::ProjectHandle* projectHandle,
                                                             const Genex0dInputData & inData) :
  DataAccess::Interface::PointAdsorptionHistory(projectHandle, nullptr)
{
  m_x = inData.xCoord;
  m_y = inData.yCoord;
  m_formationName = inData.formationName;
  m_mangledFormationName = utilities::mangle(m_formationName);
  m_fileName = inData.nodeHistoryFileName;
}

Genex0dPointAdsorptionHistory::~Genex0dPointAdsorptionHistory()
{
}

} // namespace genex0d
