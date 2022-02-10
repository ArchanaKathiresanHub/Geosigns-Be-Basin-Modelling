#pragma once

#include <string>
#include <vector>

#include "hdf5.h"


namespace DataAccess
{

class DomainShapeReader
{
public:
  DomainShapeReader(const std::string& inputHDFFileName);
  void readShape(const int numI, const int numJ, std::vector<std::vector<int>>& domainShape) const;

private:
  void extractData(const int numI, const int numJ, const hid_t groupId, const std::string& datasetName, std::vector<std::vector<int>>& domainShape) const;
  std::string getDataSetName(const hid_t groupId) const;

  std::string m_fileName;
};

}

