//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ModelPseudo1dProjectManager.h"

#include "ModelPseudo1dCommonDefinitions.h"
#include "ModelPseudo1dInputData.h"
#include "ModelPseudo1dInputMapProperties.h"
#include "ModelPseudo1dTableProperty.h"

#include "ProjectHandle.h"

#include "DataMiningObjectFactory.h"
#include "DataMiningProjectHandle.h"

#include "cmbAPI.h"
#include "ErrorHandler.h"
#include "MapsManager.h"

#include "ConstantsNumerical.h"
#include "LogHandler.h"

#include <string>
#include <vector>

namespace modelPseudo1d
{

ModelPseudo1dProjectManager::ModelPseudo1dProjectManager(const ModelPseudo1dInputData & inputData) :
  m_projectFileName{inputData.projectFilename},
  m_outProjectFileName{inputData.outProjectFilename},
  m_xCoord{inputData.xCoord},
  m_yCoord{inputData.yCoord},
  m_indI{0},
  m_indJ{0},
  m_objectFactory{nullptr},
  m_projectHandle{nullptr},
  m_mdl{nullptr},
  m_referredTablesSet{}
{
  try
  {
    m_objectFactory = new DataAccess::Mining::ObjectFactory();

    if (m_projectFileName.empty())
    {
      throw modelPseudo1d::ModelPseudo1dException() << "Empty project name!";
    }
    m_projectHandle = dynamic_cast<DataAccess::Mining::ProjectHandle*>
        (DataAccess::Interface::OpenCauldronProject(m_projectFileName, m_objectFactory));
    setModel();
  }
  catch (const ErrorHandler::Exception & ex)
  {
    cleanup();
    throw ex;
  }
  catch (const modelPseudo1d::ModelPseudo1dException  & ex)
  {
    cleanup();
    throw ex;
  }
}

void ModelPseudo1dProjectManager::cleanup()
{
  delete m_objectFactory;
  delete m_projectHandle;
}

ModelPseudo1dProjectManager::~ModelPseudo1dProjectManager()
{
  cleanup();
}

void ModelPseudo1dProjectManager::getXYIndices()
{
  const DataAccess::Interface::Grid * gridHighResolution(m_projectHandle->getHighResolutionOutputGrid());
  if (gridHighResolution->getGridPoint(m_xCoord, m_yCoord, m_indI, m_indJ))
  {
    return;
  }
  double originX = 0.0;
  double originY = 0.0;
  m_mdl->origin(originX, originY);
  double lengthX = 0.0;
  double lengthY = 0.0;
  m_mdl->arealSize(lengthX, lengthY);
  throw modelPseudo1d::ModelPseudo1dException() << "The input (X,Y) location is out of range! \n"
                                                << "Valid X ranges: (" << originX << ", " << originX + lengthX << ") \n"
                                                << "Valid Y ranges: (" << originY << ", " << originY + lengthY << ") \n";
}

void ModelPseudo1dProjectManager::setModel()
{
  m_mdl.reset(new mbapi::Model);
  if (ErrorHandler::NoError != m_mdl->loadModelFromProjectFile(m_projectFileName.c_str()))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "Cannot load model from " << m_projectFileName << ", " << m_mdl->errorMessage()
        << "\n" << ", " << m_mdl->errorMessage();
  }
}

void ModelPseudo1dProjectManager::checkSubSampling()
{
  long dXSubsampling = 1;
  long dYSubsampling = 1;
  if (ErrorHandler::NoError != m_mdl->subsampling(dXSubsampling, dYSubsampling))
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "Could not get subsampling values from project file " << m_projectFileName << ", " << m_mdl->errorMessage();
  }

  if (dXSubsampling == 1 && dYSubsampling == 1 )
  {
    return;
  }

  LogHandler(LogHandler::WARNING_SEVERITY) << "\n The model is subsampled! Subsampling values will be reset to 1.0. \n";
  if (ErrorHandler::NoError != m_mdl->setSubsampling((long)1, (long)1))
  {
    throw  ErrorHandler::Exception(m_mdl->errorCode()) << "Resetting of the subsampling was not successful, " << m_mdl->errorMessage();
  }
}

void ModelPseudo1dProjectManager::setSingleCellWindowXY()
{
  if (ErrorHandler::NoError != m_mdl->setWindow(m_indI, m_indI + 1, m_indJ, m_indJ + 1)) // TODO: this is 2X2 grid, to be fixed to 1X1 grid after fastcauldron has capability to runs with 1X1 grid.
  {
    throw ErrorHandler::Exception(m_mdl->errorCode())
        << "Windowing around the specified location failed, " << m_mdl->errorMessage();
  }
}

void ModelPseudo1dProjectManager::setReferredTablesSet()
{
  for (int row = 0; row < m_mdl->tableSize("GridMapIoTbl"); ++row)
  {
    m_referredTablesSet.insert(m_mdl->tableValueAsString("GridMapIoTbl", row, "ReferredBy"));
  }
}

ModelPseudo1dProjectManager::ConstIteratorReferredTableSet ModelPseudo1dProjectManager::referredTableConstIteratorBegin() const
{
  return m_referredTablesSet.begin();
}

ModelPseudo1dProjectManager::ConstIteratorReferredTableSet ModelPseudo1dProjectManager::referredTableConstIteratorend() const
{
  return m_referredTablesSet.end();
}

std::shared_ptr<mbapi::Model> ModelPseudo1dProjectManager::mdl() const
{
  return m_mdl;
}

unsigned int ModelPseudo1dProjectManager::indJ() const
{
  return m_indJ;
}

unsigned int ModelPseudo1dProjectManager::indI() const
{
  return m_indI;
}

} // namespace modelPseudo1d
