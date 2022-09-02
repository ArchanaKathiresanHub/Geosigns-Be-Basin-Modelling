//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "NamesValuesList.h"

#include "view/components/helpLabel.h"

#include <QLabel>

namespace casaWizard
{

namespace ua
{

NamesValuesList::NamesValuesList(const QString& listHeader):
   m_listLayout(new QGridLayout()),
   m_listHeader(listHeader)
{
   setMargin(3);
   addLayout(m_listLayout);
}

void NamesValuesList::displayList(const QVector<double>& optimalValues, const QStringList& parameterNames)
{
   clearList();
   QLabel* header = new QLabel(m_listHeader);
   header->setStyleSheet("font-weight: bold");

   QHBoxLayout* headerlayout = new QHBoxLayout();
   headerlayout->addWidget(header);

   if (m_helpLabelText.size() != 0)
   {
      HelpLabel* m_helpLabel = new HelpLabel(header,m_helpLabelText);
      m_listWidgets.push_back(m_helpLabel);

      headerlayout->addWidget(m_helpLabel);
      headerlayout->addStretch(1);
   }

   m_listWidgets.push_back(header);
   insertLayout(0,headerlayout);

   for (int i = 0; i < parameterNames.size(); i++)
   {
      QLabel* label = new QLabel(parameterNames[i] + ":");
      QLabel* value = new QLabel(QString::number(optimalValues[i], 'g', 4));
      m_listWidgets.push_back(label);
      m_listWidgets.push_back(value);

      m_listLayout->addWidget(label, i,0);
      m_listLayout->addWidget(value, i,1);
   }
}

void NamesValuesList::addToolTipToHeader(const QString& helpLabelText)
{
   m_helpLabelText = helpLabelText;
}

void NamesValuesList::clearList()
{
   if (count() > 1)
   {
      removeItem(itemAt(0));
   }

   for (int i = 0; i < m_listLayout->rowCount(); i++)
   {
      m_listLayout->removeItem(m_listLayout->itemAtPosition(i,1));
      m_listLayout->removeItem(m_listLayout->itemAtPosition(i,0));
   }

   for (auto w : m_listWidgets)
   {
      delete w;
   }
   m_listWidgets.resize(0);
}

} //ua
} //casaWizard
