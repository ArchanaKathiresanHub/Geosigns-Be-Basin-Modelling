#include "mainWindow.h"

#include "initresources.h"
#include "logDisplay.h"
#include "menuBar.h"
#include "view/components/customtab.h"
#include "view/lithoTypeBackgroundMapping.h"

#include <QStatusBar>
#include <QVBoxLayout>

namespace casaWizard
{

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  tabWidget_{new CustomTab(this)},
  logDisplay_{new LogDisplay(this)}
{
  InitResources::initialise();
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
  layout->addWidget(tabWidget_);
  layout->addWidget(logDisplay_);
}

MainWindow::~MainWindow()
{
  LithoTypeBackgroundMapping::deleteInstance();
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
