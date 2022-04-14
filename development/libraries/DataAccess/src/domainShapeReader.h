#pragma once

#include <string>
#include <vector>

#include "hdf5.h"


namespace DataAccess
{

struct Window
{
   Window(const int windowMinI, const int windowMaxI, const int windowMinJ, const int windowMaxJ) :
      minI{windowMinI},
      maxI{windowMaxI},
      minJ{windowMinJ},
      maxJ{windowMaxJ}
   {}

   int minI = 0;
   int maxI = 0;
   int minJ = 0;
   int maxJ = 0;
};

class DomainShapeReader
{
public:
  DomainShapeReader(const std::string& inputHDFFileName);
  void readShape(const Window& window, std::vector<std::vector<int> >& domainShape) const;

private:
  void extractData(const Window& window, const hid_t groupId, const std::string& datasetName, std::vector<std::vector<int>>& domainShape) const;
  std::string getDataSetName(const hid_t groupId) const;

  std::string m_fileName;
};

}

