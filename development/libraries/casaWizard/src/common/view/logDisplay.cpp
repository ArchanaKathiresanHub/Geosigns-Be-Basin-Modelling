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
   pushHideLog_{new QPushButton(this)},
   pushShowLog_{new QPushButton(this)},
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

   layout->setMargin(0);
   setLayout(layout);

   updateDisplayState();
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
      emit resizeLog(displayState_);
   }
}

void LogDisplay::showMore()
{
   if (displayState_ < 2)
   {
      displayState_++;
      updateDisplayState();
      emit resizeLog(displayState_);
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
         pushHideLog_->hide();
         pushShowLog_->setText("Show log ");
         break;
      case 1:
         lineEditLog_->show();
         pushHideLog_->show();
         pushShowLog_->show();
         pushHideLog_->setText("Hide log ");
         pushShowLog_->setText("Show more");
         break;
      case 2:
         pushHideLog_->show();
         pushShowLog_->hide();
         pushHideLog_->setText("Show less");
         break;
      default:
         lineEditLog_->show();
         break;
   }
}

} // namespace casaWizard
