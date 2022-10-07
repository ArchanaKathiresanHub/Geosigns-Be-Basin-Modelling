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
  void showLess();
  void showMore();

  QPushButton* pushClearLog() const;
  QPushButton* pushHideLog() const;
  QPushButton* pushShowLog() const;
  QTextEdit* lineEditLog() const;

signals:
  void resizeLog(int sizeIdx);

private:
  void updateDisplayState();
  QPushButton* pushClearLog_;
  QPushButton* pushHideLog_;
  QPushButton* pushShowLog_;
  QTextEdit* lineEditLog_;

  int displayState_ = 1;
};

} // namespace casaWizard
