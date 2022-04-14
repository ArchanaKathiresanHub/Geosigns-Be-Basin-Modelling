#pragma once

#include <QString>
#include <QVector>
#include <QMap>

namespace casaWizard
{

namespace ua
{

class DoeOption
{
public:
  DoeOption();
  virtual ~DoeOption() = default;
  virtual QString name() const = 0;
  virtual bool hasCalculatedDesignPoints() const = 0;
  virtual void calculateNDesignPoints(const int nInfluentialParameters) = 0;
  virtual void setArbitraryNDesignPoints(const int nArbitraryDesignPoints) = 0;
  int nDesignPoints() const;

  static QVector<DoeOption*> getDoeOptions();

protected:
  void setNDesignPoints(const int nDesignPoints);

private:
  int nDesignPoints_;
};

} // namespace ua

} // namespace casaWizard
