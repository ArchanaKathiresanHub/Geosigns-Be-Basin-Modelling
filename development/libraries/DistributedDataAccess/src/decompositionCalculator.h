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
  DecompositionCalculator(const std::vector<std::vector<int>>& domainShape, const int numCores, const int numIGlobal, const int numJGlobal,
                          const int lowResI = -1, const int lowResJ = -1);
  bool calculateDecomposition(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);
  static bool calculateStaticDecomposition(int M, int N, int& mSelected, int& nSelected, int numberOfCores);

private:
  bool addOneMoreRowToTheCore(const int numberOfValidDomainNodesForCurrentCore, const int numberOfValidDomainNodesInRow, const double averageValidNodesPerCore,
                              const int currentRow, const int coresLeft);
  void calculateCellSizes(std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);
  bool calculateDynamicDecomposition(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ);
  void calculateMinMaxNumberOfRows();
  void calculateNumberOfValidGridPoints();
  bool cellSizesAreValid(const std::vector<int>& cellSizesILocal, const std::vector<int>& cellSizesJLocal);
  bool fallbackToStaticDecomposition(int& m, int& n);
  int getNumberOfRowsForCurrentCore(size_t& currentRow, const double averageValidNodesPerCore, const int coresLeft);
  int getNumberOfValidDomainNodesInRow(const size_t currentRow);
  bool lowResDefinedAndValid();
  size_t numberOfLocalNodesI();
  size_t numberOfLocalNodesJ();
  bool percentageValidNodesHigherThan80();
  void rotateShape();

  std::vector<std::vector<int>> m_domainShape;
  const int m_numberOfCores;
  int m_totalNumberOfRemainingValidNodes;
  int m_minimumNumberOfRowsPerCore;
  int m_maximumNumberOfRowsPerCore;
  double m_maxPercentageDeviationFromAverage;
  int m_lowResIGlobal;
  int m_lowResJGlobal;
  int m_numIGlobal;
  int m_numJGlobal;
};

}

}


