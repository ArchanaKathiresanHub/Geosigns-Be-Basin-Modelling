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

ModelPseudo1dProjectManager::ModelPseudo1dProjectManager(const std::string projectFileName, const std::string outProjectFileName ):
  m_projectFileName{projectFileName},
  m_outProjectFileName{outProjectFileName},
  m_objectFactory{nullptr},
  m_projectHandle{nullptr},
  m_mdl{nullptr}
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

std::shared_ptr<mbapi::Model> ModelPseudo1dProjectManager::mdl() const
{
  return m_mdl;
}

void ModelPseudo1dProjectManager::save() const
 {
  LogHandler(LogHandler::INFO_SEVERITY) << "Saving to output file ...";

  m_mdl->saveModelToProjectFile(m_outProjectFileName.c_str());
  LogHandler(LogHandler::INFO_SEVERITY) << "ModelPseudo1d was successfully saved to output project file: "
                                        << m_outProjectFileName;
}


} // namespace modelPseudo1d
