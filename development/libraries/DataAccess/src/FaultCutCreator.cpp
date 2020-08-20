//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultCutCreator.h"

#include <algorithm>
#include <cmath>

using namespace DataAccess;
using namespace Interface;


FaultCutCreator::FaultCutCreator(const std::vector<Point>& intersections)
{
  for ( const Point& intersection : intersections )
  {
    m_faultCuts.push_back({intersection});
  }
}

FaultCutCreator::~FaultCutCreator()
{
}

std::vector<PointSequence> FaultCutCreator::createFaultCuts(const double splitDistance)
{
  bool continueMerging = true;
  while( continueMerging )
  {
    continueMerging = mergeFaultCuts( splitDistance );

    // Additional ordering operations
    sweepLeftRight();
    splitCrossing();
  }

  closeFaultCutLoops( splitDistance );
  splitCrossing();

  return m_faultCuts;
}

bool FaultCutCreator::mergeFaultCuts( const double splitDistance )
{
  std::vector<std::pair<unsigned int, Orientation>> bestMerge;

  unsigned int fc = 0;
  for ( const PointSequence& faultCut : m_faultCuts )
  {
    const Point& front = faultCut.front();
    const Point& back = faultCut.back();

    double closestDistance = 1e99;
    Orientation orientation = frontToFront;
    unsigned int bestFaultCutCandidate = 0;
    unsigned int fc1 = 0;
    for ( const PointSequence& otherfaultCut : m_faultCuts )
    {
      if ( fc1 != fc )
      {
        const Point& front1 = otherfaultCut.front();
        const Point& back1 = otherfaultCut.back();

        const double s1 = separationDistance( front, front1 ); if ( s1 < closestDistance ) { closestDistance = s1; orientation = frontToFront; bestFaultCutCandidate = fc1; }
        const double s2 = separationDistance( front, back1  ); if ( s2 < closestDistance ) { closestDistance = s2; orientation = frontToBack; bestFaultCutCandidate = fc1; }
        const double s3 = separationDistance( back,  front1 ); if ( s3 < closestDistance ) { closestDistance = s3; orientation = backToFront; bestFaultCutCandidate = fc1; }
        const double s4 = separationDistance( back,  back1  ); if ( s4 < closestDistance ) { closestDistance = s4; orientation = backToBack; bestFaultCutCandidate = fc1; }
      }
      fc1++;
    }

    if ( closestDistance > splitDistance )
    {
      bestFaultCutCandidate = fc;
    }
    bestMerge.push_back({bestFaultCutCandidate, orientation});
    fc++;
  }

  std::vector<unsigned int> remove;
  unsigned int iCurrentFault = 0;
  for ( const std::pair<unsigned int, Orientation>& b : bestMerge )
  {
    const unsigned int iOtherFault = b.first;
    if ( bestMerge[iOtherFault].first == iCurrentFault && iOtherFault != iCurrentFault )
    {
      PointSequence& otherFaultCut = m_faultCuts[iOtherFault];
      PointSequence& mergedFaultCut = m_faultCuts[iCurrentFault];

      switch (b.second)
      {
        case frontToFront:
          for ( const Point& p : otherFaultCut )
          {
            mergedFaultCut.insert(mergedFaultCut.begin(), p);
          }
          break;
        case frontToBack:
          for ( int i = otherFaultCut.size() - 1; i>=0; --i )
          {
            mergedFaultCut.insert(mergedFaultCut.begin(), otherFaultCut[i]);
          }
          break;
        case backToFront:
          for ( const Point& p : otherFaultCut )
          {
            mergedFaultCut.push_back(p);
          }
          break;
        case backToBack:
          for ( int i = otherFaultCut.size() - 1; i>=0; --i )
          {
            mergedFaultCut.push_back(otherFaultCut[i]);
          }
          break;
      }

      otherFaultCut.clear();
      remove.push_back(iOtherFault);
      bestMerge[iOtherFault].first = iOtherFault;
    }
    ++iCurrentFault;
  }

  std::sort(remove.begin(), remove.end(), std::greater<unsigned int>());
  for ( unsigned int r : remove )
  {
    m_faultCuts.erase(m_faultCuts.begin() + r);
  }

  return !remove.empty() && m_faultCuts.size() > 1;
}

void FaultCutCreator::sweepLeftRight()
{
  for ( PointSequence& orderedFaultCut : m_faultCuts )
  {
    // Sweep from start to end
    for (int currentIndex = 1; currentIndex < orderedFaultCut.size(); currentIndex++)
    {
      const int betterPlacement = findBetterIndexLowerThanCurrent(currentIndex, orderedFaultCut);

      if (betterPlacement != -1)
      {
        Point pointToBeTransfered = orderedFaultCut[currentIndex];
        orderedFaultCut.erase(orderedFaultCut.begin() + currentIndex);
        orderedFaultCut.emplace(orderedFaultCut.begin() + betterPlacement, pointToBeTransfered);
      }
    }

    // Sweep from end to start
    for (int currentIndex = orderedFaultCut.size() - 2; currentIndex >= 0; currentIndex--)
    {
      const int betterPlacement = findBetterIndexHigherThanCurrent(currentIndex, orderedFaultCut);

      if (betterPlacement != -1)
      {
        Point pointToBeTransfered = orderedFaultCut[currentIndex];
        orderedFaultCut.emplace(orderedFaultCut.begin() + betterPlacement, pointToBeTransfered);
        orderedFaultCut.erase(orderedFaultCut.begin() + currentIndex);
      }
    }
  }
}

int FaultCutCreator::findBetterIndexLowerThanCurrent(const unsigned int currentIndex, const PointSequence& orderedFaultCut) const
{
  double currentDistance;
  if (currentIndex == orderedFaultCut.size() - 1)
  {
    currentDistance = separationDistance(orderedFaultCut[currentIndex-1], orderedFaultCut[currentIndex]);
  }
  else
  {
    currentDistance = separationDistance(orderedFaultCut[currentIndex-1], orderedFaultCut[currentIndex])
        + separationDistance(orderedFaultCut[currentIndex], orderedFaultCut[currentIndex+1])
        - separationDistance(orderedFaultCut[currentIndex - 1], orderedFaultCut[currentIndex+1]);
  }

  // Distance when the point would be placed at the start
  unsigned int newBestIndex = 0;
  double newBestDistance = separationDistance(orderedFaultCut[currentIndex], orderedFaultCut.front());
  double tryDistance;

  // Try all other positions smaller than the actual position and find the optimum
  for (unsigned int i = 1; i < currentIndex; i++)
  {
    tryDistance = separationDistance(orderedFaultCut[i-1], orderedFaultCut[currentIndex])
        + separationDistance(orderedFaultCut[currentIndex], orderedFaultCut[i])
        - separationDistance(orderedFaultCut[i], orderedFaultCut[i-1]);

    if (tryDistance < newBestDistance)
    {
      newBestDistance = tryDistance;
      newBestIndex = i;
    }
  }

  if (newBestDistance < currentDistance)
  {
    return static_cast<int>(newBestIndex);
  }

  return -1;
}

int FaultCutCreator::findBetterIndexHigherThanCurrent(const unsigned int currentIndex, const PointSequence& orderedFaultCut) const
{
  double currentDistance;
  if (currentIndex == 0)
  {
    currentDistance = separationDistance(orderedFaultCut[currentIndex], orderedFaultCut[currentIndex+1]);
  }
  else
  {
    currentDistance = separationDistance(orderedFaultCut[currentIndex-1], orderedFaultCut[currentIndex])
        + separationDistance(orderedFaultCut[currentIndex], orderedFaultCut[currentIndex+1])
        - separationDistance(orderedFaultCut[currentIndex - 1], orderedFaultCut[currentIndex+1]);
  }

  // Distance when new point is placed at the end
  unsigned int newBestIndex = orderedFaultCut.size();
  double newBestDistance = separationDistance(orderedFaultCut[currentIndex], orderedFaultCut.back());
  double tryDistance = 1e99;

  // Try all other positions larger than the actual position and find the optimum
  for (unsigned int i = currentIndex + 2; i < orderedFaultCut.size() - 1; i++)
  {
    tryDistance = separationDistance(orderedFaultCut[i-1], orderedFaultCut[currentIndex])
        + separationDistance(orderedFaultCut[currentIndex], orderedFaultCut[i])
        - separationDistance(orderedFaultCut[i], orderedFaultCut[i-1]);

    if (tryDistance < newBestDistance)
    {
      newBestDistance = tryDistance;
      newBestIndex = i;
    }
  }

  if (newBestDistance < currentDistance)
  {
    return static_cast<int>(newBestIndex);
  }

  return -1;
}

void FaultCutCreator::splitCrossing()
{
  for ( PointSequence& faultCut : m_faultCuts )
  {
    for ( int i = 1; i + 2 < faultCut.size(); ++i )
    {
      const Point& p1 = faultCut[i-1];
      const Point& p2 = faultCut[i];
      const double dx1 = p1(X_COORD) - p2(X_COORD);
      const double dy1 = p1(Y_COORD) - p2(Y_COORD);

      for ( int j = i+2; j < faultCut.size(); ++j )
      {
        const Point& p3 = faultCut[j-1];
        const Point& p4 = faultCut[j];

        // Check intersection using cross products, if both point p3 and p4 are on the same side of segment p1-p2 continue as there is no intersection.
        const double s1 = dx1*(p3(Y_COORD) - p1(Y_COORD)) - dy1*(p3(X_COORD) - p1(X_COORD));
        const double s2 = dx1*(p4(Y_COORD) - p1(Y_COORD)) - dy1*(p4(X_COORD) - p1(X_COORD));
        if ( s1 * s2 >= 0.0 ) continue;

        // Check intersection using cross products, if both point p1 and p2 are on the same side of segment p3-p4 continue as there is no intersection.
        const double dx2 = p3(X_COORD) - p4(X_COORD);
        const double dy2 = p3(Y_COORD) - p4(Y_COORD);
        const double s3 = dx2*(p1(Y_COORD) - p3(Y_COORD)) - dy2*(p1(X_COORD) - p3(X_COORD));
        const double s4 = dx2*(p2(Y_COORD) - p3(Y_COORD)) - dy2*(p2(X_COORD) - p3(X_COORD));
        if ( s3 * s4 >= 0.0 ) continue;

        // Intersection found, now flip the section between the crossing segments to unwind the cross.
        PointSequence midSection;
        for ( int k = i; k < j; ++k )
        {
          midSection.push_back(faultCut[k]);
        }

        int m = midSection.size()  - 1;
        for ( int k = i; k < j; ++k )
        {
          faultCut[k] = midSection[m--];
        }

        splitCrossing();
        return;
      }
    }
  }
}

void FaultCutCreator::closeFaultCutLoops( const double splitDistance )
{
  for (PointSequence& faultCut : m_faultCuts)
  {
    if (separationDistance(faultCut.front(), faultCut.back()) < splitDistance && faultCut.size() > 2)
    {
      faultCut.push_back(faultCut.front());
    }
  }
}
