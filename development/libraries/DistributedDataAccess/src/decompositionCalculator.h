//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

namespace DataAccess
{
namespace Interface
{

class DecompositionCalculator
{
public:
  DecompositionCalculator(const std::vector<std::vector<int>>& domainShape, const int numCores, double maxPercentageDeviation = 1e10);
  void calculateDomain(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);

private:
  std::vector<std::vector<int>> domainShape_;
  const int numberOfCores_;
  int totalNumberOfRemainingValidNodes_;
  int minimumNumberOfRowsPerCore_;
  int maximumNumberOfRowsPerCore_;
  double maxPercentageDeviation_;

  int getNumberOfValidDomainNodesInRow(const size_t currentRow);
  void calculateNumberOfValidNodes();
  int getNumberOfRowsForCurrentCore(size_t& currentRow, const double averageValidNodesPerCore, const int coresLeft);
  void rotateShape();
  void calculateMinMaxNumberOfRows();
  void calculateCellSizes(std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);

  size_t numberOfNodesI();
  size_t numberOfNodesJ();
  bool addOneMoreRowToTheCore(const int numberOfValidDomainNodesForCurrentCore, const int numberOfValidDomainNodesInRow, const double averageValidNodesPerCore,
                              const int currentRow, const int coresLeft);
};

}

}


