#include "mainWindow.h"

#include "initresources.h"
#include "logDisplay.h"
#include "menuBar.h"
#include "view/sharedComponents/customtab.h"
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

   QSizePolicy sizePolicyNew(QSizePolicy::Expanding, QSizePolicy::Expanding);
   sizePolicyNew.setHorizontalStretch(1);
   sizePolicyNew.setVerticalStretch(1);
   sizePolicyNew.setHeightForWidth(sizePolicy().hasHeightForWidth());
   setSizePolicy(sizePolicyNew);

   m_mainLayout = new QVBoxLayout(centralWidget);
   m_mainLayout->addWidget(tabWidget_,3);
   m_mainLayout->addWidget(logDisplay_,1);
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

QVBoxLayout* MainWindow::mainLayout() const
{
   return m_mainLayout;
}

void MainWindow::slotResizeLog(int size)
{
   switch (size)
   {
   case 0:
      m_mainLayout->setStretch(0,1);
      m_mainLayout->setStretch(1,0);
      break;
   case 1:
      m_mainLayout->setStretch(0,3);
      m_mainLayout->setStretch(1,1);
      break;
   case 2:
      m_mainLayout->setStretch(0,1);
      m_mainLayout->setStretch(1,10);
      break;
   }

}

} // namespace casaWizard
