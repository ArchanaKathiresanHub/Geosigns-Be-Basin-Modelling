//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "decompositionCalculator.h"

#include <climits>
#include <cmath>

namespace DataAccess
{
  namespace Interface
  {

    DecompositionCalculator::DecompositionCalculator(const std::vector<std::vector<int>>& domainShape, const int numCores, double maxPercentageDeviation) :
      domainShape_{domainShape},
      numberOfCores_{numCores},
      totalNumberOfRemainingValidNodes_{0},
      minimumNumberOfRowsPerCore_{1},
      maximumNumberOfRowsPerCore_{INT_MAX},
      maxPercentageDeviation_{maxPercentageDeviation}
    {
    }

    void DecompositionCalculator::calculateDomain(int& m, int& n, std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
    {
      const bool needToRotate = numberOfNodesI() < numberOfNodesJ();
      if (needToRotate)
      {
        rotateShape();
      }

      calculateMinMaxNumberOfRows();
      calculateCellSizes(cellSizesI, cellSizesJ);

      if (needToRotate) // Switch the variables, so they match the domain after rotation
      {
        m = 1;
        n = numberOfCores_;

        const std::vector<int> tmp = cellSizesI;
        cellSizesI = cellSizesJ;
        cellSizesJ = tmp;
      }
      else
      {
        m = numberOfCores_;
        n = 1;
      }
    }

    void DecompositionCalculator::rotateShape()
    {
      std::vector<std::vector<int>> rotatedShape;
      for (size_t i = 0; i < domainShape_[0].size(); i++)
      {
        std::vector<int> transposedRow;
        for (size_t j = 0; j < domainShape_.size(); j++)
        {
          transposedRow.push_back(domainShape_[j][i]);
        }

        rotatedShape.push_back(transposedRow);
      }

      domainShape_ = rotatedShape;
    }

    void DecompositionCalculator::calculateMinMaxNumberOfRows()
    {
      double averageNumberOfRowsPerCore = (double)numberOfNodesI() / numberOfCores_;
      minimumNumberOfRowsPerCore_ = std::floor(averageNumberOfRowsPerCore * (100 - maxPercentageDeviation_) / 100);
      if (minimumNumberOfRowsPerCore_ < 1) minimumNumberOfRowsPerCore_ = 1;

      maximumNumberOfRowsPerCore_ = std::ceil(averageNumberOfRowsPerCore * (100 + maxPercentageDeviation_) / 100);
      if (maximumNumberOfRowsPerCore_ > numberOfNodesI()) maximumNumberOfRowsPerCore_ = numberOfNodesI();
    }

    void DecompositionCalculator::calculateCellSizes(std::vector<int>& cellSizesI, std::vector<int>& cellSizesJ)
    {
      cellSizesJ.push_back(numberOfNodesJ());

      calculateNumberOfValidNodes();
      size_t currentRow = 0;
      for (int iCore = 0; iCore < numberOfCores_ - 1; iCore++)
      {
        double averageRemainingValidNodesPerRemainingCore = totalNumberOfRemainingValidNodes_ / (numberOfCores_ - iCore);
        cellSizesI.push_back(getNumberOfRowsForCurrentCore(currentRow, averageRemainingValidNodesPerRemainingCore, numberOfCores_ - iCore));
      }
      cellSizesI.push_back(numberOfNodesI() - currentRow); // Remainder is for the last cell
    }

    void DecompositionCalculator::calculateNumberOfValidNodes()
    {
      for (const auto& row : domainShape_)
      {
        for (const auto& value : row)
        {
          totalNumberOfRemainingValidNodes_ += value;
        }
      }
    }

    int DecompositionCalculator::getNumberOfRowsForCurrentCore(size_t& currentRow, const double averageValidNodesPerCore, const int coresLeft)
    {
      int numberOfValidDomainNodesForCurrentCore = 0;
      int numberOfRowsForCurrentCore = 0;
      for (; currentRow < numberOfNodesI(); currentRow++)
      {
        if (numberOfRowsForCurrentCore == maximumNumberOfRowsPerCore_)
        {
          totalNumberOfRemainingValidNodes_ -= numberOfValidDomainNodesForCurrentCore;
          break;
        }

        const int numberOfValidDomainNodesInRow = getNumberOfValidDomainNodesInRow(currentRow);

        if ((numberOfValidDomainNodesForCurrentCore + numberOfValidDomainNodesInRow >= averageValidNodesPerCore && (numberOfRowsForCurrentCore >= minimumNumberOfRowsPerCore_)))
        {
          if (((std::fabs(numberOfValidDomainNodesForCurrentCore + numberOfValidDomainNodesInRow - averageValidNodesPerCore) <
              std::fabs(numberOfValidDomainNodesForCurrentCore - averageValidNodesPerCore) &&
                static_cast<double>((numberOfNodesI() - currentRow)) / (coresLeft) >= minimumNumberOfRowsPerCore_)))
          {
            numberOfRowsForCurrentCore++;
            currentRow++;
            numberOfValidDomainNodesForCurrentCore += numberOfValidDomainNodesInRow;
          }

          totalNumberOfRemainingValidNodes_ -= numberOfValidDomainNodesForCurrentCore;
          break;
        }

        numberOfRowsForCurrentCore++;
        numberOfValidDomainNodesForCurrentCore += numberOfValidDomainNodesInRow;
      }

      return numberOfRowsForCurrentCore;
    }

    int DecompositionCalculator::getNumberOfValidDomainNodesInRow(const size_t currentRow)
    {
      int numberOfValidDomainNodesInRow = 0;
      for (size_t j = 0; j < numberOfNodesJ(); j++)
      {
        numberOfValidDomainNodesInRow += domainShape_[currentRow][j];
      }

      return numberOfValidDomainNodesInRow;
    }

    size_t DecompositionCalculator::numberOfNodesI()
    {
      return domainShape_.size();
    }

    size_t DecompositionCalculator::numberOfNodesJ()
    {
      return domainShape_[0].size();
    }
  }
}
