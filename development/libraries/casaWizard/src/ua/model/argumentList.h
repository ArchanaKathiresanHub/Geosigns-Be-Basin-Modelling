// The arguments of an influential parameter
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace ua
{

class ArgumentList
{
public:
  enum class Type { Double, StringList };

  ArgumentList() = default;
  void addArgument(const QString& name, const double value);
  void addArgument(const QString& name, const QStringList& values);

  int size() const;
  QString getName(const int index) const;
  Type getType(const int index) const;
  double getDoubleArgument(const int index) const;
  QString getSelectedValue(const int index) const;
  QStringList getListOptions(const int index) const;
  QString toString() const;

  void setDoubleArguments(const QVector<double> values);
  void setListValues(const QStringList& values);
  void setListOptions(const int index, const QStringList& options);

  QString write() const;
  void read(const int version, const QString& input);

private:

  QStringList names_;
  QVector<int> indices_;
  QVector<Type> types_;
  QVector<double> doubleArguments_;
  QVector<int> selectedList_;
  QVector<QStringList> listOptions_;
};

} // namespace ua

} // namespace casaWizard
