// Main window for applications, with general menubar, tabs and logger
#pragma once

#include <QMainWindow>

namespace casaWizard
{

class LogDisplay;
class MenuBar;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget* parent = 0);
  virtual ~MainWindow();

  QTabWidget* tabWidget() const;
  LogDisplay* logDisplay() const;
  virtual MenuBar* menu() const = 0;

private:
  QTabWidget* tabWidget_;
  LogDisplay* logDisplay_;
};

} // namespace casaWizard
