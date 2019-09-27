#include "argumentList.h"

#include "model/scenarioIO.h"

namespace casaWizard
{

namespace ua
{

QStringList convertToQStringList(const QVector<int>& vec);
QStringList convertToQStringList(const QVector<double>& vec);
QStringList convertToQStringList(const QVector<ArgumentList::Type>& vec);
QStringList convertToQStringList(const QVector<QStringList>& vec);

void ArgumentList::addArgument(const QString& name, const double value)
{
  names_.append(name);
  doubleArguments_.append(value);
  types_.append(Type::Double);
  indices_.append(doubleArguments_.size()-1);
}

void ArgumentList::addArgument(const QString& name, const QStringList& values)
{
  names_.append(name);
  listOptions_.append(values);
  types_.append(Type::StringList);
  indices_.append(listOptions_.size()-1);
  selectedList_.append(0);
}

int ArgumentList::size() const
{
  return names_.size();
}

QString ArgumentList::getName(const int index) const
{
  return names_[index];
}

ArgumentList::Type ArgumentList::getType(const int index) const
{
  return types_[index];
}

double ArgumentList::getDoubleArgument(const int index) const
{
  return doubleArguments_[indices_[index]];
}

QString ArgumentList::getSelectedValue(const int index) const
{
  const int indexInList = selectedList_[indices_[index]];
  return getListOptions(index)[indexInList];
}

QStringList ArgumentList::getListOptions(const int index) const
{
  return listOptions_[indices_[index]];
}

QString ArgumentList::toString() const
{
  QString argumentString{""};
  const int nArgs = names_.size();

  for (int i = 0; i<nArgs; ++i)
  {
    switch (types_[i])
    {
      case Type::Double:
      {
        argumentString += QString::number(getDoubleArgument(i)) + " ";
        break;
      }
      case  Type::StringList:
      {

        argumentString += "\"" + getSelectedValue(i) + "\" ";
        break;
      }
    }
  }

  return argumentString.trimmed();
}

void ArgumentList::setDoubleArguments(const QVector<double> values)
{
  if (doubleArguments_.size() != values.size())
  {
    return;
  }

  doubleArguments_ = values;
}

void ArgumentList::setListValues(const QStringList& values)
{
  const int nValues = values.size();
  const int nLists = listOptions_.size();
  if (nValues != nLists)
  {
    return;
  }
  for (int i = 0; i<nValues; ++i)
  {
    const int indexInList = listOptions_[i].indexOf(values[i]);
    selectedList_[i] = indexInList;
    if (indexInList<0)
    {
      selectedList_[i] = 0;
    }
  }
}

void ArgumentList::setListOptions(const int index, const QStringList& options)
{
  if (options.empty())
  {
    return;
  }
  if (index < listOptions_.length())
  {
    listOptions_[index] = options;
    selectedList_[index] = 0;
  }
}

QString ArgumentList::write() const
{
  QVector<QStringList> output;
  output.append(names_);
  output.append(convertToQStringList(indices_));
  output.append(convertToQStringList(types_));
  output.append(convertToQStringList(doubleArguments_));
  output.append(convertToQStringList(selectedList_));
  output.append(convertToQStringList(listOptions_));
  QString out;
  for (const QStringList& l : output)
  {
    out += l.join(scenarioIO::subListSeparator) + scenarioIO::listSeparator;
  }
  return out;
}

void ArgumentList::read(const int /*version*/, const QString& input)
{
  const QStringList in = input.split(scenarioIO::listSeparator);

  names_ = in[0].split(scenarioIO::subListSeparator);

  indices_.clear();
  for (const QString& s : in[1].split(scenarioIO::subListSeparator))
  {
    indices_.append(s.toInt());
  }

  types_.clear();
  for (const QString& s : in[2].split(scenarioIO::subListSeparator))
  {
    if (s=="double")
    {
      types_.append(Type::Double);
    }
    else
    {
      types_.append(Type::StringList);
    }
  }

  doubleArguments_.clear();
  for (const QString& s : in[3].split(scenarioIO::subListSeparator))
  {
    doubleArguments_.append(s.toDouble());
  }

  selectedList_.clear();
  for (const QString& s : in[4].split(scenarioIO::subListSeparator))
  {
    selectedList_.append(s.toInt());
  }

  listOptions_.clear();
  for (const QString& s : in[5].split(scenarioIO::subListSeparator))
  {
    listOptions_.append(s.split(scenarioIO::subSubListSeparator));
  }
}

QStringList convertToQStringList(const QVector<int>& vec)
{
  QStringList out;
  for( const int i : vec)
  {
    out << QString::number(i);
  }
  return out;
}

QStringList convertToQStringList(const QVector<double>& vec)
{
  QStringList out;
  for( const double d : vec)
  {
    out << scenarioIO::doubleToQString(d);
  }
  return out;
}

QStringList convertToQStringList(const QVector<ArgumentList::Type>& vec)
{
  QStringList out;
  for(const ArgumentList::Type& t : vec)
  {
    switch(t)
    {
      case ArgumentList::Type::Double:
        out << "double";
        break;
      case ArgumentList::Type::StringList:
        out << "list";
        break;
      default:
        out << "";
    }
  }
  return out;
}

QStringList convertToQStringList(const QVector<QStringList>& vec)
{
  QStringList out;
  for (const QStringList& l : vec)
  {
    out << l.join(scenarioIO::subSubListSeparator);
  }
  return out;
}

} // namespace ua

} // namespace casaWizard
