#include "lithofraction.h"

#include "model/scenarioIO.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

const QStringList Lithofraction::percentNames = {"Percent1", "Percent2", "Percent3", "None"};

Lithofraction::Lithofraction(const QString& layerName,
                             const int firstComponent,
                             const double minPercentageFirstComponent,
                             const double maxPercentageFirstComponent,
                             const int secondComponent,
                             const double minFractionSecondComponent,
                             const double maxFractionSecondComponent) :
  layerName_{layerName},
  firstComponent_{firstComponent},
  minPercentageFirstComponent_{minPercentageFirstComponent},
  maxPercentageFirstComponent_{maxPercentageFirstComponent},
  secondComponent_{secondComponent},
  minFractionSecondComponent_{minFractionSecondComponent},
  maxFractionSecondComponent_{maxFractionSecondComponent}
{
}

int Lithofraction::version() const
{
  return 0;
}

Lithofraction Lithofraction::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 7)
  {
    return Lithofraction("UnknownFromRead", 0, 0, 0, 0, 0, 0);
  }

  return Lithofraction
  {
    p[0],
    p[1].toInt(),
    p[3].toDouble(),
    p[4].toDouble(),
    p[2].toInt(),
    p[5].toDouble(),
    p[6].toDouble()
  };
}

QStringList Lithofraction::write() const
{
  QStringList out;
  out << layerName_
      << QString::number(firstComponent_)
      << QString::number(secondComponent_)
      << scenarioIO::doubleToQString(minPercentageFirstComponent_)
      << scenarioIO::doubleToQString(maxPercentageFirstComponent_)
      << scenarioIO::doubleToQString(minFractionSecondComponent_)
      << scenarioIO::doubleToQString(maxFractionSecondComponent_);
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

} // namespace sac

} // namespace casaWizard
