//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "cmbAPI.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace DataAccess
{
namespace Mining
{
class ObjectFactory;
class ProjectHandle;
} // namespace Mining
} // namespace DataAccess

namespace mbapi
{
class Model;
} // namespace mbapi

namespace modelPseudo1d
{

struct ModelPseudo1dInputData;

class ModelPseudo1dProjectManager
{
  typedef std::unordered_set<std::string> UnorderedStringSet;
public:
  typedef UnorderedStringSet::const_iterator ConstIteratorReferredTableSet;

  explicit ModelPseudo1dProjectManager(const ModelPseudo1dInputData & inputData);
  ~ModelPseudo1dProjectManager();

  void checkSubSampling();
  void getXYIndices();
  void setReferredTablesSet();
  unsigned int indI() const;
  unsigned int indJ() const;

  //mbapi::Model& mdl() const;

  ConstIteratorReferredTableSet referredTableConstIteratorBegin() const;
  ConstIteratorReferredTableSet referredTableConstIteratorend() const;

  void setSingleCellWindowXY();

private:
  void cleanup();
  void setModel();

  const std::string m_projectFileName;
  const std::string m_outProjectFileName;
  const double m_xCoord;
  const double m_yCoord;
  unsigned int m_indI;
  unsigned int m_indJ;

  DataAccess::Mining::ObjectFactory * m_objectFactory;
  DataAccess::Mining::ProjectHandle * m_projectHandle;
  mbapi::Model& m_mdl;
  UnorderedStringSet m_referredTablesSet;
  ConstIteratorReferredTableSet m_referredTableConstIterator;
};

} // namespace modelPseudo1d
