#pragma once

#include <QLabel>

namespace casaWizard
{


class HelpLabel : public QLabel
{
  Q_OBJECT
public:
  HelpLabel(QWidget *parent = nullptr, const QString& tooltipText = "");
};

}
