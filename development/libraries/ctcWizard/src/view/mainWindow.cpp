#include "mainWindow.h"

#include "ctcTab.h"

#include <QAction>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QDesktopWidget>

namespace ctcWizard
{

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	setupUi(); resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
}

void MainWindow::setupUi()
{
  setObjectName(QStringLiteral("MainWindow"));
  setWindowTitle("CTC Wizard:"+QString(CTCUI_VERSION));
  QSizePolicy sizePolicyNew(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicyNew.setHorizontalStretch(1);
  sizePolicyNew.setVerticalStretch(1);
  sizePolicyNew.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(sizePolicyNew);

  actionExit_ = new QAction("Exit", this);
  actionExit_->setObjectName(QStringLiteral("actionExit"));

  setMenuBar(new QMenuBar(this));
  setStatusBar(new QStatusBar(this));
  QMenu* menuFile = new QMenu("File", menuBar());
  menuFile->setObjectName(QStringLiteral("menuFile"));

  menuBar()->addAction(menuFile->menuAction());
  menuFile->addAction(actionExit_);

  QWidget* centralWidget = new QWidget(this);
  centralWidget->setObjectName(QStringLiteral("centralWidget"));
  setCentralWidget(centralWidget);


  tabWidget_ = new QTabWidget(centralWidget);
  tabWidget_->setObjectName(QStringLiteral("tabWidget"));

  pushClearLog_ = new QPushButton("Clear log", centralWidget);
  pushClearLog_->setObjectName(QStringLiteral("pushClearLog"));

  lineEditLog_ = new QTextEdit(centralWidget);
  lineEditLog_->setObjectName(QStringLiteral("lineEditLog"));
  lineEditLog_->setEnabled(true);

  QHBoxLayout* horizontalLayout_3 = new QHBoxLayout();
  horizontalLayout_3->addWidget(pushClearLog_);
  horizontalLayout_3->addWidget(new QWidget(centralWidget));
  horizontalLayout_3->setStretch(1,1);

  QVBoxLayout* verticalLayout_8 = new QVBoxLayout(centralWidget);
  verticalLayout_8->setSpacing(6);
  verticalLayout_8->addWidget(tabWidget_);
  verticalLayout_8->addLayout(horizontalLayout_3);
  verticalLayout_8->addWidget(lineEditLog_);
  verticalLayout_8->setStretch(0,3);
  verticalLayout_8->setStretch(2,1);


  ctcTab_ = new CTCtab();

  tabWidget_->addTab(ctcTab_, "CTC Input Parameters");
}

const QAction* MainWindow::actionExit() const
{
  return actionExit_;
}

const QPushButton* MainWindow::pushClearLog() const
{
  return pushClearLog_;
}

QTextEdit* MainWindow::lineEditLog() const
{
  return lineEditLog_;
}

const QTabWidget* MainWindow::tabWidget() const
{
  return tabWidget_;
}

CTCtab* MainWindow::ctcTab() const
{
  return ctcTab_;
}


} // namespace ctcWizard
