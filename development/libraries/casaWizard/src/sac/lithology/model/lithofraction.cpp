//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofraction.h"

#include "model/scenarioIO.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

const QStringList Lithofraction::percentNames = {"Percent1", "Percent2", "Percent3", "None"};

Lithofraction::Lithofraction(const QString& layerName,
                             const int firstComponent,
                             const double minPercentageFirstComponent,
                             const double maxPercentageFirstComponent,
                             const int secondComponent,
                             const double minFractionSecondComponent,
                             const double maxFractionSecondComponent,
                             const bool doFirstOptimization,
                             const bool doSecondOptimization) :
  layerName_{layerName},
  firstComponent_{firstComponent},
  minPercentageFirstComponent_{minPercentageFirstComponent},
  maxPercentageFirstComponent_{maxPercentageFirstComponent},
  secondComponent_{secondComponent},
  minFractionSecondComponent_{minFractionSecondComponent},
  maxFractionSecondComponent_{maxFractionSecondComponent},
  doFirstOptimization_{doFirstOptimization},
  doSecondOptimization_{doSecondOptimization}
{
}

int Lithofraction::version() const
{
  return 1;
}

Lithofraction Lithofraction::read(const int version, const QStringList& p)
{
  if (version == 0 &&
      p.size() == 7)
  {       
    Lithofraction lithofraction
    {
      p[0],
      p[1].toInt(),
      p[3].toDouble(),
      p[4].toDouble(),
      p[2].toInt(),
      p[5].toDouble(),
      p[6].toDouble(),
    };
    if (lithofraction.secondComponent() == 3)
    {
      lithofraction.setDoSecondOptimization(false);
      lithofraction.setSecondComponent(3 - lithofraction.firstComponent());
    }
    return lithofraction;
  }

  if (p.size() == 9)
  {
    return Lithofraction
    {
      p[0],
      p[1].toInt(),
      p[3].toDouble(),
      p[4].toDouble(),
      p[2].toInt(),
      p[5].toDouble(),
      p[6].toDouble(),
      p[7] == "true",
      p[8] == "true",
    };
  }

  return Lithofraction("UnknownFromRead", 0, 0, 0, 0, 0, 0);
}



QStringList Lithofraction::write() const
{
  QStringList out;
  QString firstOptimization = doFirstOptimization_ ? "true" : "false";
  QString secondOptimization = doSecondOptimization_ ? "true" : "false";

  out << layerName_
      << QString::number(firstComponent_)
      << QString::number(secondComponent_)
      << scenarioIO::doubleToQString(minPercentageFirstComponent_)
      << scenarioIO::doubleToQString(maxPercentageFirstComponent_)
      << scenarioIO::doubleToQString(minFractionSecondComponent_)
      << scenarioIO::doubleToQString(maxFractionSecondComponent_)
      << firstOptimization
      << secondOptimization;

  return out;
}

QString Lithofraction::layerName() const
{
  return layerName_;
}

void Lithofraction::setLayerName(const QString& value)
{
  layerName_ = value;
}
int Lithofraction::firstComponent() const
{
  return firstComponent_;
}

void Lithofraction::setFirstComponent(int value)
{
  firstComponent_ = value;
}
double Lithofraction::minPercentageFirstComponent() const
{
  return minPercentageFirstComponent_;
}

void Lithofraction::setMinPercentageFirstComponent(double value)
{
  minPercentageFirstComponent_ = value;
}
double Lithofraction::maxPercentageFirstComponent() const
{
  return maxPercentageFirstComponent_;
}

void Lithofraction::setMaxPercentageFirstComponent(double value)
{
  maxPercentageFirstComponent_ = value;
}
int Lithofraction::secondComponent() const
{
  return secondComponent_;
}

void Lithofraction::setSecondComponent(int value)
{
  secondComponent_ = value;
}
double Lithofraction::minFractionSecondComponent() const
{
  return minFractionSecondComponent_;
}

void Lithofraction::setMinFractionSecondComponent(double value)
{
  minFractionSecondComponent_ = value;
}
double Lithofraction::maxFractionSecondComponent() const
{
  return maxFractionSecondComponent_;
}

void Lithofraction::setMaxFractionSecondComponent(double value)
{
  maxFractionSecondComponent_ = value;
}

int Lithofraction::thirdComponent() const
{
  // Three componenets, i.e., 0, 1 and 2 (sum = 3)
  return 3 - firstComponent_ - secondComponent_;
}

bool Lithofraction::doFirstOptimization() const
{
  return doFirstOptimization_;
}

void Lithofraction::setDoFirstOptimization(bool doFirstOptimization)
{
  doFirstOptimization_ = doFirstOptimization;
}

bool Lithofraction::doSecondOptimization() const
{
  return doSecondOptimization_;
}

void Lithofraction::setDoSecondOptimization(bool doSecondOptimization)
{
  doSecondOptimization_ = doSecondOptimization;
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
