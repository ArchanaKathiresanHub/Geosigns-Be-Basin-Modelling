#include "doeOptionImpl.h"

#include <cmath>
#include <stdexcept>

namespace casaWizard
{

namespace ua
{

DoeOption::DoeOption() :
  nDesignPoints_{0}
{
}

void DoeOption::setNDesignPoints(const int nDesignPoints)
{
  nDesignPoints_ = nDesignPoints;
}

int DoeOption::nDesignPoints() const
{
  return nDesignPoints_;
}

QVector<DoeOption*> DoeOption::getDoeOptions()
{
  QVector<DoeOption*> doeOptions;

  doeOptions.push_back(new DoeTornado());
  doeOptions.push_back(new DoeBoxBehnken());
  doeOptions.push_back(new DoePlackettBurman());
  doeOptions.push_back(new DoePlackettBurmanMirror());
  doeOptions.push_back(new DoeFullFactorial());
  doeOptions.push_back(new DoeSpaceFilling());
  doeOptions.push_back(new DoeUserDefined());

  return doeOptions;
}

// ---------------------------------------------------------------------------

DoeTornado::DoeTornado() :
  DoeOption()
{
}

QString DoeTornado::name() const
{
  return QString("Tornado");
}

bool DoeTornado::hasCalculatedDesignPoints() const
{
  return true;
}

void DoeTornado::calculateNDesignPoints(const int nInfluentialParameters)
{
  if (nInfluentialParameters == 0)
  {
    setNDesignPoints(0);
  }
  else
  {
    setNDesignPoints(2*nInfluentialParameters + 1);
  }
}

void DoeTornado::setArbitraryNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

// ---------------------------------------------------------------------------

DoeBoxBehnken::DoeBoxBehnken() :
  DoeOption()
{
}

QString DoeBoxBehnken::name() const
{
  return QString("BoxBehnken");
}

bool DoeBoxBehnken::hasCalculatedDesignPoints() const
{
  return true;
}

void DoeBoxBehnken::calculateNDesignPoints(const int nInfluentialParameters)
{
  if (nInfluentialParameters == 0)
  {
    setNDesignPoints(0);
  }
  else if (nInfluentialParameters == 6 || nInfluentialParameters == 7)
  {
    setNDesignPoints(1 + 8*nInfluentialParameters);
  }
  else if (nInfluentialParameters == 9)
  {
    setNDesignPoints(97);
  }
  else if (nInfluentialParameters > 9 && nInfluentialParameters < 13)
  {
    setNDesignPoints(1 + 16*nInfluentialParameters);
  }
  else
  {
    setNDesignPoints(1 + 2*nInfluentialParameters*(nInfluentialParameters - 1));
  }
}

void DoeBoxBehnken::setArbitraryNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

// ---------------------------------------------------------------------------

DoePlackettBurman::DoePlackettBurman() :
  DoeOption()
{
}

QString DoePlackettBurman::name() const
{
  return QString("PlackettBurman");
}

bool DoePlackettBurman::hasCalculatedDesignPoints() const
{
  return true;
}

void DoePlackettBurman::calculateNDesignPoints(const int nInfluentialParameters)
{
  // Note: in SUMlib there is also an option for adding a center point. This option is not used in Cauldron.
  // Therefore, it is not considered here.

  if (nInfluentialParameters >= 48 || nInfluentialParameters == 0)
  {
    setNDesignPoints(0);
  }
  else if (nInfluentialParameters == 1)
  {
    setNDesignPoints(2);
  }
  else
  {
    setNDesignPoints(4*(nInfluentialParameters/4 + 1));
  }
}

void DoePlackettBurman::setArbitraryNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

// ---------------------------------------------------------------------------

DoePlackettBurmanMirror::DoePlackettBurmanMirror() :
  DoeOption()
{
}

QString DoePlackettBurmanMirror::name() const
{
  return QString("PlackettBurmanMirror");
}

bool DoePlackettBurmanMirror::hasCalculatedDesignPoints() const
{
  return true;
}

void DoePlackettBurmanMirror::calculateNDesignPoints(const int nInfluentialParameters)
{
  // Note: in SUMlib there is also an option for adding a center point. This option is not used in Cauldron.
  // Therefore, it is not considered here.

  if (nInfluentialParameters >= 48)
  {
    throw std::runtime_error("Number of influential parameters cannot be greater than 47 for PlacketBurmanMirror doe option.");
  }

  if (nInfluentialParameters == 0)
  {
    setNDesignPoints(0);
  }
  else if (nInfluentialParameters == 1)
  {
    setNDesignPoints(2);
  }
  else if (nInfluentialParameters == 2)
  {
    setNDesignPoints(4*(nInfluentialParameters/4 + 1));
  }
  else
  {
    setNDesignPoints(8*(nInfluentialParameters/4 + 1));
  }
}

void DoePlackettBurmanMirror::setArbitraryNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

// ---------------------------------------------------------------------------

DoeFullFactorial::DoeFullFactorial() :
  DoeOption()
{
}

QString DoeFullFactorial::name() const
{
  return QString("FullFactorial");
}

bool DoeFullFactorial::hasCalculatedDesignPoints() const
{
  return true;
}

void DoeFullFactorial::calculateNDesignPoints(const int nInfluentialParameters)
{
  if (nInfluentialParameters == 0)
  {
    setNDesignPoints(0);
  }
  else
  {
    setNDesignPoints(int(std::pow(2, nInfluentialParameters)) + 1);
  }
}

void DoeFullFactorial::setArbitraryNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

// ---------------------------------------------------------------------------

DoeSpaceFilling::DoeSpaceFilling() :
  DoeOption()
{
}

QString DoeSpaceFilling::name() const
{
  return QString("SpaceFilling");
}

bool DoeSpaceFilling::hasCalculatedDesignPoints() const
{
  return false;
}

void DoeSpaceFilling::calculateNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

void DoeSpaceFilling::setArbitraryNDesignPoints(const int nArbitraryDesignPoints)
{
  setNDesignPoints(nArbitraryDesignPoints);
}

// ---------------------------------------------------------------------------

DoeUserDefined::DoeUserDefined() :
  DoeOption()
{
}

QString DoeUserDefined::name() const
{
  return QString("UserDefined");
}

bool DoeUserDefined::hasCalculatedDesignPoints() const
{
  return true;
}

void DoeUserDefined::calculateNDesignPoints(const int /*nArbitraryDesignPoints*/)
{
  // Nothing to be done.
}

void DoeUserDefined::setArbitraryNDesignPoints(const int nArbitraryDesignPoints)
{
  setNDesignPoints(nArbitraryDesignPoints);
}


} // namespace ua

} // namespace casaWizard
