#include "logDisplay.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>

namespace casaWizard
{

LogDisplay::LogDisplay(QWidget* parent) :
  QWidget(parent),
  pushClearLog_{new QPushButton("Clear log", this)},
  lineEditLog_{new QTextEdit(this)}
{
  lineEditLog_->setEnabled(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(pushClearLog_);
  buttonLayout->addWidget(new QWidget(this), 1);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addLayout(buttonLayout);
  layout->addWidget(lineEditLog_);

  setLayout(layout);
}

void LogDisplay::addLog(const QString &logMessage)
{
  lineEditLog_->append(logMessage);
}

void LogDisplay::clearLog()
{
  lineEditLog_->setText("");
}

QPushButton* LogDisplay::pushClearLog() const
{
  return pushClearLog_;
}

QTextEdit* LogDisplay::lineEditLog() const
{
  return lineEditLog_;
}

} // namespace casaWizard
