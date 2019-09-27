// Widget which contains a text field with the logs and a button to clear the log
#pragma once

#include <QWidget>

class QPushButton;
class QTextEdit;

namespace casaWizard
{

class LogDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit LogDisplay(QWidget* parent = 0);

  void addLog(const QString& logMessage);
  void clearLog();

  QPushButton* pushClearLog() const;
  QTextEdit* lineEditLog() const;

private:
  QPushButton* pushClearLog_;
  QTextEdit* lineEditLog_;

};

} // namespace casaWizard
