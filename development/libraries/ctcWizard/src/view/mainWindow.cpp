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
#include <QDir>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDirIterator>

namespace ctcWizard
{
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
	{
		if (qgetenv("ADVANCED_USER").isEmpty()) {

			qputenv("ADVANCED_USER", "No");

		}
		setupUi();
		resize(QDesktopWidget().availableGeometry(this).size() * 0.7);
		connect(ctcTab_->lineEditProject3D(), SIGNAL(textChanged(const QString&)), this, SLOT(slotLineEditProject3dTextChanged(const QString&)));
	}

	void MainWindow::slotLineEditProject3dTextChanged(const QString& project3dPath) {
		m_project3dPath = project3dPath;
#ifdef FUTURE_USE
		ctcTab_->disableProject3dSelection();
#endif
	}

	// Clearing the CTC-Workspace when the CTC-UI is closed
	void MainWindow::closeEvent(QCloseEvent* event)
	{
		// The following ifdef-part is kept for future reference
		// This part invokes a confirmation pop-up on closing CTC-UI
#ifdef FUTURE_USE
		QMessageBox::StandardButton resBtn = QMessageBox::question(this,
			"CTC Wizard" + QString(CTCUI_VERSION),
			tr("Do you want to clear the CTC working directory and exit?\n"),
			QMessageBox::No | QMessageBox::Yes,
			QMessageBox::Yes
		);

		if (resBtn != QMessageBox::Yes) {
			event->accept();
		}
		else
#endif
		{
			// Checking whether the user has selected a project3d file or not
			
			if (m_project3dPath.isEmpty()) {
				qDebug("\n No project3d file is selected.\n");
			}
			else {
				// Notifying the user about the workspace clean-up in the terminal since 
				// it is not visible in the log once the application is closed
				qDebug("\n CTC-Workspace is being cleared.\n");

				// Getting the path of the CTC-Workspace based on the selected project3d file location
				QFileInfo info(m_project3dPath);
				QDir dir = info.dir().absolutePath();
				QString cwd = dir.absolutePath();
				QDir baseDir(cwd);

				// Filtering out the directories in the CTC-Workspace
				dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

				// Removing all the log files from the CTC-workspace
				QDir delLog(cwd, { "*.log" });
				for (const QString& logFilename : delLog.entryList()) {
					delLog.remove(logFilename);
					qDebug("\n clearing log files.\n");
				}

				if (!QString(qgetenv("ADVANCED_USER")).compare("No", Qt::CaseInsensitive))
				{
					// Removing recursively all the directories from the CTC-Workspace
					for (const QString& dirName : dir.entryList()) {
						bool validate = false;
						bool toDelete = false;
						auto val = dirName.toInt(&validate);
						QString delFolderPath = baseDir.path().append("/" + dirName);
						if (validate) {
							QDirIterator it(delFolderPath, QDirIterator::Subdirectories);
							// Iterate through the directory using the QDirIterator
							while (it.hasNext()) {
								QString filename = it.next();
								QFileInfo file(filename);

								if (file.isDir()) { // Check if it's a dir
									continue;
								}
								// If the filename contains target string - put it in the hitlist
								if (file.fileName().contains("CrustalThicknessCalculator_Results.HDF", Qt::CaseInsensitive)) {
									//hit
									toDelete = true; break;
								}
							}
							if (toDelete) {
								QDir delFolder(delFolderPath);
								delFolder.removeRecursively();
							}
						}

					}

					auto delPATH =  m_project3dPath.split(".project3d")[0] + "_CauldronOutputDir";
					QDir PTdir(delPATH);
					auto success = PTdir.removeRecursively();
					qDebug("\n Clearing CTC created directories.\n");
				}
				else
					qDebug("\n CTC directories not deleted for ADVANCED_USER.\n");
			}
		}
	}

	void MainWindow::setupUi()
	{
		setObjectName(QStringLiteral("MainWindow"));
		setWindowTitle("CTC Wizard:" + QString(CTCUI_VERSION));
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
		horizontalLayout_3->setStretch(1, 1);

		QVBoxLayout* verticalLayout_8 = new QVBoxLayout(centralWidget);
		verticalLayout_8->setSpacing(6);
		verticalLayout_8->addWidget(tabWidget_);
		verticalLayout_8->addLayout(horizontalLayout_3);
		verticalLayout_8->addWidget(lineEditLog_);
		verticalLayout_8->setStretch(0, 3);
		verticalLayout_8->setStretch(2, 1);


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
