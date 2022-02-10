//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "decompositionCalculator.h"

#include "LogHandler.h"

#include <climits>
#include <cmath>
#include <iostream>

namespace DataAccess
{
  namespace Interface
  {

    DecompositionCalculator::DecompositionCalculator(const std::vector<std::vector<int>>& domainShape, const int numCores, const int numIGlobal, const int numJGlobal,
                                                     const int lowResI, const int lowResJ) :
      m_domainShape{domainShape},
      m_numberOfCores{numCores},
      m_totalNumberOfRemainingValidNodes{0},
      m_minimumNumberOfRowsPerCore{2},
      m_maximumNumberOfRowsPerCore{INT_MAX},
      m_maxPercentageDeviationFromAverage{-1},
      m_lowResIGlobal{lowResI},
      m_lowResJGlobal{lowResJ},
      m_numIGlobal{numIGlobal},
      m_numJGlobal{numJGlobal}
    {
      char* dynamicDecomposition = getenv("DYNAMIC_DECOMPOSITION_MAX_DEVIATION");
      if (dynamicDecomposition)
      {
        std::string dynamicDecompositionString(dynamicDecomposition);
        try
        {
          m_maxPercentageDeviationFromAverage = std::stod(dynamicDecompositionString);
        }
        catch (...)
        {
        }
      }

      LogHandler(LogHandler::DEBUG_SEVERITY) << "The Maximum deviation used for dynamic decomposition = " << m_maxPercentageDeviationFromAverage;
    }

    bool DecompositionCalculator::calculateDecomposition(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
    {
      if (!calculateDynamicDecomposition(m, n, cellSizesI, cellSizesJ))
      {
        return fallbackToStaticDecomposition(m, n);
      }

      return true;
    }

    bool DecompositionCalculator::calculateDynamicDecomposition(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
    {
      if (m_domainShape.empty() || percentageValidNodesHigherThan80() || m_maxPercentageDeviationFromAverage < 0)
      {
        return false;
      }

      std::vector<int> cellSizesILocal;
      std::vector<int> cellSizesJLocal;

      const bool needToRotate = numberOfLocalNodesI() < numberOfLocalNodesJ();
      if (needToRotate)
      {
        rotateShape();
      }

      calculateMinMaxNumberOfRows();
      calculateCellSizes(cellSizesILocal, cellSizesJLocal);

      if (needToRotate) // Switch the variables, so they match the domain after rotation
      {
        m = 1;
        n = m_numberOfCores;

        const std::vector<int> tmp = cellSizesILocal;
        cellSizesILocal = cellSizesJLocal;
        cellSizesJLocal = tmp;
      }
      else
      {
        m = m_numberOfCores;
        n = 1;
      }

      if (lowResDefinedAndValid())
      {
        if (!cellSizesAreValid(cellSizesILocal, cellSizesJLocal))
        {
          return false;
        }
      }

      cellSizesI = cellSizesILocal;
      cellSizesJ = cellSizesJLocal;

      return true;
    }

    void DecompositionCalculator::rotateShape()
    {
      std::vector<std::vector<int>> rotatedShape;
      for (size_t i = 0; i < m_domainShape[0].size(); i++)
      {
        std::vector<int> transposedRow;
        for (size_t j = 0; j < m_domainShape.size(); j++)
        {
          transposedRow.push_back(m_domainShape[j][i]);
        }

        rotatedShape.push_back(transposedRow);
      }

      m_domainShape = rotatedShape;
    }

    void DecompositionCalculator::calculateMinMaxNumberOfRows()
    {
      double averageNumberOfRowsPerCore = (double)numberOfLocalNodesI() / m_numberOfCores;
      int rowDeviation = std::floor(averageNumberOfRowsPerCore * m_maxPercentageDeviationFromAverage/ 100.0);
      m_minimumNumberOfRowsPerCore = std::floor(averageNumberOfRowsPerCore) - rowDeviation;
      m_maximumNumberOfRowsPerCore = std::ceil(averageNumberOfRowsPerCore) + rowDeviation;

      if (m_minimumNumberOfRowsPerCore < 2) m_minimumNumberOfRowsPerCore = 2;
      if (m_maximumNumberOfRowsPerCore > numberOfLocalNodesI()) m_maximumNumberOfRowsPerCore = numberOfLocalNodesI();
    }

    void DecompositionCalculator::calculateCellSizes(std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
    {
      cellSizesJ.push_back(numberOfLocalNodesJ());

      calculateNumberOfValidGridPoints();
      size_t currentRow = 0;
      for (int iCore = 0; iCore < m_numberOfCores - 1; iCore++)
      {
        double averageRemainingValidNodesPerRemainingCore = m_totalNumberOfRemainingValidNodes / (m_numberOfCores - iCore);
        cellSizesI.push_back(getNumberOfRowsForCurrentCore(currentRow, averageRemainingValidNodesPerRemainingCore, m_numberOfCores - iCore));
      }
      cellSizesI.push_back(numberOfLocalNodesI() - currentRow); // Remainder is for the last cell
    }

    bool DecompositionCalculator::percentageValidNodesHigherThan80()
    {
      double totalNumberOfValidNodes = 0.0;
      for (const auto& row : m_domainShape)
      {
        for (const auto& value : row)
        {
          totalNumberOfValidNodes += value;
        }
      }

      double totalNumberOfNodes = m_numIGlobal*m_numJGlobal;
      double percentageValidNodes = totalNumberOfValidNodes/totalNumberOfNodes * 100;

      LogHandler(LogHandler::DEBUG_SEVERITY) << "Percentage of valid nodes = " << percentageValidNodes;

      return percentageValidNodes > 80;
    }

    void DecompositionCalculator::calculateNumberOfValidGridPoints()
    {
      for (const auto& row : m_domainShape)
      {
        for (const auto& value : row)
        {
          m_totalNumberOfRemainingValidNodes += value;
        }
      }
    }

    int DecompositionCalculator::getNumberOfRowsForCurrentCore(size_t& currentRow, const double averageValidNodesPerCore, const int coresLeft)
    {
      int numberOfValidDomainGridpointsForCurrentCore = 0;
      int numberOfRowsForCurrentCore = 0;
      for (; currentRow < numberOfLocalNodesI(); currentRow++)
      {
        if (numberOfRowsForCurrentCore == m_maximumNumberOfRowsPerCore)
        {
          m_totalNumberOfRemainingValidNodes -= numberOfValidDomainGridpointsForCurrentCore;
          break;
        }

        const int numberOfValidDomainNodesInRow = getNumberOfValidDomainNodesInRow(currentRow);
        int TestAverageNumberofRowsLeft = static_cast<double>(numberOfLocalNodesI() - currentRow)/(coresLeft-1);
        if (TestAverageNumberofRowsLeft >= m_maximumNumberOfRowsPerCore)
        {
          numberOfRowsForCurrentCore++;
          numberOfValidDomainGridpointsForCurrentCore += numberOfValidDomainNodesInRow;
          continue;
        }

        if (numberOfValidDomainGridpointsForCurrentCore + numberOfValidDomainNodesInRow >= averageValidNodesPerCore && (numberOfRowsForCurrentCore >= m_minimumNumberOfRowsPerCore))
        {
          if (((std::fabs(numberOfValidDomainGridpointsForCurrentCore + numberOfValidDomainNodesInRow - averageValidNodesPerCore) <
              std::fabs(numberOfValidDomainGridpointsForCurrentCore - averageValidNodesPerCore)
                &&
                static_cast<double>((numberOfLocalNodesI() - currentRow)) / (coresLeft) >= m_minimumNumberOfRowsPerCore)))
          {
            numberOfRowsForCurrentCore++;
            currentRow++;
            numberOfValidDomainGridpointsForCurrentCore += numberOfValidDomainNodesInRow;
          }

          m_totalNumberOfRemainingValidNodes -= numberOfValidDomainGridpointsForCurrentCore;
          break;
        }

        numberOfRowsForCurrentCore++;
        numberOfValidDomainGridpointsForCurrentCore += numberOfValidDomainNodesInRow;
      }

      return numberOfRowsForCurrentCore;
    }

    int DecompositionCalculator::getNumberOfValidDomainNodesInRow(const size_t currentRow)
    {
      int numberOfValidDomainNodesInRow = 0;
      for (size_t j = 0; j < numberOfLocalNodesJ(); j++)
      {
        numberOfValidDomainNodesInRow += m_domainShape[currentRow][j];
      }

      return numberOfValidDomainNodesInRow;
    }

    size_t DecompositionCalculator::numberOfLocalNodesI()
    {
      return m_domainShape.size();
    }

    size_t DecompositionCalculator::numberOfLocalNodesJ()
    {
      return m_domainShape[0].size();
    }

    bool DecompositionCalculator::cellSizesAreValid(const std::vector<int>& cellSizesILocal,  const std::vector<int>& cellSizesJLocal)
    {
      for (const int cellSize : cellSizesILocal)
      {
        if (cellSize/(m_numIGlobal/m_lowResIGlobal) < 2)
        {
          return false;
        }
      }
      for (const int cellSize : cellSizesJLocal)
      {
        if (cellSize/(m_numJGlobal/m_lowResJGlobal) < 2)
        {
           return false;
        }
      }

      return true;
    }

    bool DecompositionCalculator::lowResDefinedAndValid()
    {
      return (m_lowResIGlobal > 0 && m_lowResJGlobal > 0) && (m_lowResIGlobal != m_numIGlobal || m_lowResJGlobal != m_numJGlobal);
    }

    bool DecompositionCalculator::fallbackToStaticDecomposition(int& m, int& n)
    {
      bool checkHighRes = calculateStaticDecomposition(m_numIGlobal, m_numJGlobal, m, n, m_numberOfCores);
      bool checkLowRes = true;
      if (m_lowResIGlobal > 0 && m_lowResJGlobal > 0)
      {
        checkLowRes = calculateStaticDecomposition(m_lowResIGlobal, m_lowResJGlobal, m, n, m_numberOfCores);
      }

      return checkHighRes && checkLowRes;
    }

    bool DecompositionCalculator::calculateStaticDecomposition(int M, int N, int & mSelected, int & nSelected, int numberOfCores)
    {
      double minimumScalingRatio = 1e10;
      bool scalingFound = false;
      int m, n;
      for (m = numberOfCores; m > 0; --m)
      {
        n = numberOfCores / m;
        if (m * n == numberOfCores && m <= M && n <= N)
        {
          scalingFound = true;
          double mScaling = double(M) / double (m);
          double nScaling = double(N) / double (n);

          double scalingRatio = std::max (mScaling, nScaling) / std::min (mScaling, nScaling);
          if (scalingRatio < minimumScalingRatio)
          {
            mSelected = m;
            nSelected = n;
            minimumScalingRatio = scalingRatio;
          }
        }
      }

       return scalingFound;
    }

  }
}
