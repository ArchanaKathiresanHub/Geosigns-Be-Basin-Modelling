// Interface for influential parameters
#pragma once

#include "argumentList.h"

namespace casaWizard
{

class ProjectReader;

namespace ua
{

class InfluentialParameter
{
public:
  explicit InfluentialParameter(const QString& name, const QString& casaName, const QString& label, const QString& unitSI, const int totalNumberOfParameters);
  int version() const;

  virtual QString label() const;
  virtual QString nameArgumentBased() const;
  QString casaName() const;
  QString name() const;
  QString labelName() const;
  QString unitSI() const;
  virtual void fillArguments(const ProjectReader& projectReader) = 0;

  int totalNumberOfParameters() const;
  ArgumentList& arguments();
  const ArgumentList& arguments() const;

  static InfluentialParameter* createFromIndex(int index);
  static InfluentialParameter* createFromList(const int version, const QStringList& list);
  static QStringList list();

  QStringList write() const;
  void read(const int version, const QString& input);

protected:
  void addPDFArgument();

private:
  enum class Type
  {
    TopCrustHeatProduction,
    TopCrustHeatProductionGrid,
    TopCrustHeatProductionGridScaling,
    CrustThinning,
    CrustThinningOneEvent,
    ThermalConductivity,
    EquilibriumOceanicLithosphereThickness,
    InitialLithosphericMantleThickness,
    SurfaceTemperature
  };

  static QVector<Type> types();
  static InfluentialParameter* create(const Type& type);

  ArgumentList arguments_;
  QString name_;
  QString casaName_;
  QString labelName_;
  QString unitSI_;
  int totalNumberOfParameters_;

};

} // namespace ua

} // namespace casaWizard
