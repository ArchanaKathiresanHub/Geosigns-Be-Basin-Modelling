// Functions to set doe options.
#pragma once

#include "doeOption.h"

#include <QVector>

namespace casaWizard
{

namespace ua
{

class DoeTornado : public DoeOption
{
public:
  DoeTornado();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nInfluentialParameters) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoeBoxBehnken : public DoeOption
{
public:
  DoeBoxBehnken();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nInfluentialParameters) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoePlackettBurman : public DoeOption
{
public:
  DoePlackettBurman();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nInfluentialParameters) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoePlackettBurmanMirror : public DoeOption
{
public:
  DoePlackettBurmanMirror();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nInfluentialParameters) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoeFullFactorial : public DoeOption
{
public:
  DoeFullFactorial();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nInfluentialParameters) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoeSpaceFilling : public DoeOption
{
public:
  DoeSpaceFilling();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

class DoeUserDefined : public DoeOption
{
public:
  DoeUserDefined();
  QString name() const override;
  bool hasCalculatedDesignPoints() const override;
  void calculateNDesignPoints(const int nArbitraryDesignPoints) override;
  void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) override;
};

} // namespace ua

} // namespace casaWizard
