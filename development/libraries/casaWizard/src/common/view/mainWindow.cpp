#include "mainWindow.h"

#include "logDisplay.h"
#include "menuBar.h"

#include <QStatusBar>
#include <QVBoxLayout>

namespace casaWizard
{

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  tabWidget_{new QTabWidget(this)},
  logDisplay_{new LogDisplay(this)}
{
  QWidget* centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  setStatusBar(new QStatusBar(this));

  resize(1600, 900);
  QSizePolicy sizePolicyNew(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicyNew.setHorizontalStretch(1);
  sizePolicyNew.setVerticalStretch(1);
  sizePolicyNew.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicyNew);

  QVBoxLayout* layout = new QVBoxLayout(centralWidget);
  layout->addWidget(tabWidget_,3);
  layout->addWidget(logDisplay_,1);
}

QTabWidget* MainWindow::tabWidget() const
{
  return tabWidget_;
}

LogDisplay* MainWindow::logDisplay() const
{
  return logDisplay_;
}

} // namespace casaWizard
