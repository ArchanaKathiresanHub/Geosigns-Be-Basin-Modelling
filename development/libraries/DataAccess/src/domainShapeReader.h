#pragma once

#include <string>
#include <vector>

namespace DataAccess
{

class DomainShapeReader
{
public:
  explicit DomainShapeReader(const std::string& inputHDFFileName);
  std::vector<std::vector<int>> readShape(const int numI, const int numJ);

private:
  std::string fileName_;
};

}

