#pragma once

#include <QMainWindow>

class QAction;
class QTabWidget;
class QTextEdit;
class QPushButton;

namespace ctcWizard
{

class CTCtab;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = 0);

  void setupUi();

  const QAction* actionExit() const;
  const QAction* actionOpenFile() const;

  const QPushButton* pushClearLog() const;
  QTextEdit* lineEditLog() const;
  const QTabWidget* tabWidget() const;

  CTCtab* ctcTab() const;

private:
  QAction* actionExit_;
  QAction* actionOpenFile_;

  QPushButton* pushClearLog_;
  QTextEdit* lineEditLog_;
  QTabWidget* tabWidget_;

  CTCtab* ctcTab_;
};

} // namespace ctcWizard
