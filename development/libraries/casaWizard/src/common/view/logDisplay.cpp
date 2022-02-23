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
  pushHideLog_{new QPushButton("Show less", this)},
  pushShowLog_{new QPushButton("Show more", this)},
  lineEditLog_{new QTextEdit(this)}
{
  lineEditLog_->setEnabled(true);

  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(pushClearLog_);
  buttonLayout->addWidget(new QWidget(this), 1);
  buttonLayout->addWidget(pushShowLog_);
  buttonLayout->addWidget(pushHideLog_);

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addLayout(buttonLayout);
  layout->addWidget(lineEditLog_);

  setLayout(layout);
}

void LogDisplay::addLog(const QString &logMessage)
{
  lineEditLog_->append(logMessage);
  lineEditLog_->update();
}

void LogDisplay::clearLog()
{
  lineEditLog_->setText("");
  lineEditLog_->update();
}

void LogDisplay::showLess()
{
  if (displayState_ > 0)
  {
    displayState_--;
    updateDisplayState();
  }
}

void LogDisplay::showMore()
{
  if (displayState_ < 2)
  {
    displayState_++;
    updateDisplayState();
  }
}

QPushButton* LogDisplay::pushClearLog() const
{
  return pushClearLog_;
}

QPushButton*LogDisplay::pushHideLog() const
{
  return pushHideLog_;
}

QPushButton*LogDisplay::pushShowLog() const
{
  return pushShowLog_;
}

QTextEdit* LogDisplay::lineEditLog() const
{
  return lineEditLog_;
}

void LogDisplay::updateDisplayState()
{
  switch (displayState_)
  {
    case 0:
      lineEditLog_->hide();
      break;
    case 1:
      lineEditLog_->show();
      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
      break;
    case 2:
      lineEditLog_->show();
      setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
      break;
  }
}

} // namespace casaWizard
