// Main window for applications, with general menubar, tabs and logger
#pragma once

#include <QMainWindow>
#include <QVBoxLayout>

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
   QVBoxLayout* mainLayout() const;
   virtual MenuBar* menu() const = 0;

public slots:
   void slotResizeLog(int size);

private:
   QTabWidget* tabWidget_;
   LogDisplay* logDisplay_;
   QVBoxLayout* m_mainLayout;
};

} // namespace casaWizard
