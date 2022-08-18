//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QVBoxLayout>
#include <QVector>

#include <memory>

class QGridLayout;

namespace casaWizard
{

class HelpLabel;

namespace ua
{

class NamesValuesList : public QVBoxLayout
{
   Q_OBJECT

public:
   NamesValuesList(const QString& listHeader);

   void addToolTipToHeader(const QString& helpLabelText);
   void displayList(const QVector<double>& optimalValues, const QStringList& parameterNames);
   void clearList();

private:

   QString m_helpLabelText;

   QGridLayout* m_listLayout;
   QVector<QWidget*> m_listWidgets;
   QString m_listHeader;
};

} //ua
} //casaWizard


