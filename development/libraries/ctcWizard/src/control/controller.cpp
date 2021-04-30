#include "controller.h"

#include "control/ctcController.h"

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QProcess>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

namespace ctcWizard
{

Controller::Controller() : QObject(),
  ui_(),
  ctcScenario_(),
  subControllers_()
{
  createConnections();

  subControllers_.append(new CTCcontroller(ui_.ctcTab(), ctcScenario_, this));
}

// Starting the CTC-UI
void Controller::showUI() {
    ui_.show();
}

void Controller::createConnections() const
{
    connect(ui_.actionExit(),     SIGNAL(triggered()), this, SLOT(slotActionExitTriggered()));
    connect(ui_.pushClearLog(),   SIGNAL(clicked()),   this, SLOT(slotPushClearLogClicked()));
}

// Closing the CTC-UI
void Controller::slotActionExitTriggered() {
    ui_.close();
}

void Controller::slotActionOpenFileTriggered()
{
    ctcScenario_.setCtcFilePathCTC(QFileDialog::getOpenFileName(ui_.centralWidget(),
        "Load CTC file",
        ctcScenario_.ctcFilePathCTC(),
        "CTC (*.ctc)"));

    QFile file(ctcScenario_.ctcFilePathCTC());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        log("Failed loading CTC file");
        return;
    }

    QTextStream in(&file);
    QString logText;
    while (!in.atEnd())
    {
        logText.append(in.readLine() + "\n");
    }

    log(logText);
}

QTextStream& qStdOut()
{
	static QTextStream ts(stdout);
	return ts;
}
/// <summary>
	/// Put a string between double quotes.
	/// </summary>
	/// <param name="value">Value to be put between double quotes ex: foo</param>
	/// <returns>double quoted string ex: "foo"</returns>
static QString AddDoubleQuotes(QString value)
{
	return "\"" + value + "\"";
}

void Controller::executeFastcauldronScript(const QString& filePath, const QString& fastcldrnRunMode, const QString numProc) const
{
    log("- Start running Cauldron");
    QTimer timer;
    timer.start(100);
    while (timer.remainingTime() > 0)
    {
        qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    QString cldrnRunMode;
    if (fastcldrnRunMode.contains("Decompaction"))
    {
        cldrnRunMode = "-decompaction -allproperties";
    }
    else if (fastcldrnRunMode.contains("Hydrostatic"))
    {
        cldrnRunMode = "-temperature";
    }
    else if (fastcldrnRunMode.contains("Iteratively Coupled"))
    {
        cldrnRunMode = "-itcoupled";
    }

    QProcess process;
    connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });

    QFileInfo info(filePath);
    process.setWorkingDirectory(info.absoluteDir().path());
    //
    QStringList strLst = info.fileName().simplified().split(".");
    QDir dir(info.dir().absolutePath() + "/" + strLst[0] + "_CauldronOutputDir");
    if (dir.exists())
    {
        log("BE WARNED YOUR PREVIOUS PRESSURE CALCULATION RUNS WILL BE DELETED!");
        dir.removeRecursively();
    }

    // Removing the fastcauldron log files of the previous run from the CTC-workspace
    log("Fastcauldron log files of the previous run will be deleted (if present)!!!");
    QDir delLog(info.dir().absolutePath(), { "*.log" });
    for (const QString& filename : delLog.entryList()) {
        delLog.remove(filename);
    }

#ifdef Q_OS_WIN

    /// Define the Usual environment Variables in the VS project and also add the following the the PATH of fastcaulron.exe
    QString CLDRN_BIN = getenv("CLDRN_BIN") + QString("\\fastcauldron.exe");
    // This path is added during MSMPI installation
    QString MPI_BIN = getenv("MSMPI_BIN") + QString("mpiexec.exe");
    auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::AUTO);

#else

    //v2021.*nightly --- is the dafault version in /apps/sssdev/ibs as on 6th March 2021
    QString CLDRN_BIN = "/apps/sssdev/share/fastcauldron";
    QString MPI_BIN = "mpirun";
    auto wlm = workloadmanagers::WorkLoadManager::Create("cldrn.sh", workloadmanagers::WorkLoadManagerType::AUTO);

#endif // WIN32

    QString runPT = QString::fromStdString
    (
        wlm->JobSubmissionCommand("cldrn", "default.q", "0:30", "ctcPressureJob", "",
            "err.log", numProc.toStdString(), "", "", process.workingDirectory().toStdString(), false, true,
            (AddDoubleQuotes(MPI_BIN).toStdString() + " -n " +
                numProc.toStdString() + ' ' + AddDoubleQuotes(CLDRN_BIN).toStdString() +
                " -project " + AddDoubleQuotes(filePath).toStdString() + " " + cldrnRunMode.toStdString())
        )
    );

    log("- The CWD is: " + process.workingDirectory());
    // does not work, because i/o redirection is handled by a shell, not by QProcess or the external exe
#ifdef Q_OS_WIN
    auto isOk = processCommand(process, runPT);
#else
    auto isOk = processShCommand(process, runPT); //bsub <myJobs.sh
#endif
    if (isOk)
        log("- Finished running Cauldron successfully!");
    else
        log("- Cauldron was unsuccessful!");

    //if (wlm) delete wlm;
}

void Controller::executeCtcScript(const QString& ctcFilenameWithPath, const QString numProc) const
{
  log("- Start running ctc");
  QTimer timer;
  timer.start(100);
  while(timer.remainingTime()>0)
  {
    qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
  }

  QProcess process;
  connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});

  QFileInfo info(ctcFilenameWithPath);
  QDir baseDir(info.absoluteDir().path());
  if(info.exists())
  {
      baseDir.cdUp();
  }
  QDir scenarioDirec(info.absoluteDir().path());

  process.setWorkingDirectory(scenarioDirec.path());

  QStringList fileList = baseDir.entryList(QStringList() << "*.*" , QDir::Files);
  foreach (QString file, fileList) 
  {
      QFileInfo fileInfo(file);
      if(!((fileInfo.completeSuffix().contains("log")) || 
          (fileInfo.completeSuffix().contains("zip"))  || 
          (fileInfo.completeSuffix().contains("sh"))   ||
          (fileInfo.completeSuffix().contains("project3d"))))
      {
          QFile::copy(baseDir.path() + "/" + file, scenarioDirec.path() + "/" + file);
      }
  }
  QStringList strList = info.fileName().simplified().split(".");


  //Create Project_CauldronOutputDir in the CTC scenario working dir to copy the fastcauldron output files
  QString fastCtcOutputDirPath = scenarioDirec.path() + "/" + strList[0] + "_CauldronOutputDir";
  QDir fastCtcOutputDir(fastCtcOutputDirPath);
  if (fastCtcOutputDir.exists())//not absolutely needed 
  {
      //remove it
      fastCtcOutputDir.removeRecursively();
  }
  else {
      //create it
      fastCtcOutputDir.mkpath(fastCtcOutputDirPath);
  }
  //Copy the fastcauldron output dir (Project_CauldronOutputDir) content to the CTC scenario working dir
  QString fastcauldronOutputDirPath = baseDir.path() + "/" + strList[0] + "_CauldronOutputDir";
  QDir fastcauldronOutputDir(fastcauldronOutputDirPath);
  
  QStringList fastcbmOutputDirFileList = fastcauldronOutputDir.entryList(QStringList() << "*.*", QDir::Files);
  foreach(QString file1, fastcbmOutputDirFileList)
  {
      if (fastcauldronOutputDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() != 0)
      {
          QFileInfo fileInfo(file1);
          QFile::copy(fastcauldronOutputDirPath + "/" + file1, fastCtcOutputDirPath + "/" + file1);
      }
      else {
          log("- fastcauldron output dir is empty");
      }
  }

  auto cwd = process.workingDirectory();

#ifdef Q_OS_WIN
  /// Define the Usual environment Variables in the VS project and also add the following the the PATH of fastctc.exe
  QString CTC_BIN = getenv("CTC_BIN") + QString("\\fastctc.exe");
  // This path is added during MSMPI installation
  QString MPI_BIN = getenv("MSMPI_BIN") + QString("mpiexec.exe");

  auto wlm = workloadmanagers::WorkLoadManager::Create(cwd.toStdString() + "/ctc.sh", workloadmanagers::WorkLoadManagerType::AUTO);
#else
  //v2021.*nightly --- is the dafault version in /apps/sssdev/ibs as on 6th March 2021
  QString CTC_BIN = "/apps/sssdev/share/fastctc";
  QString MPI_BIN = "mpirun";
  auto wlm = workloadmanagers::WorkLoadManager::Create(cwd.toStdString() + "/ctc.sh", workloadmanagers::WorkLoadManagerType::AUTO);
#endif // WIN32
  auto saveP3 = AddDoubleQuotes(
	  info.absoluteDir().path() + "/" + strList[0] + "_ctc_out.project3d").toStdString();
  QString runCTC = QString::fromStdString
  (
      wlm->JobSubmissionCommand("cldrn", "default.q", "0:30", "ctcCalcJob", "",
		  "err.log", numProc.toStdString(), "", "", process.workingDirectory().toStdString(), false, true,
		  (AddDoubleQuotes(MPI_BIN).toStdString() + " -n " +
			  numProc.toStdString() + ' ' + AddDoubleQuotes(CTC_BIN).toStdString() +
			  " -merge -project " + AddDoubleQuotes(ctcFilenameWithPath).toStdString() + (" -save ") + saveP3))
  );

  log("- The CWD is: " + process.workingDirectory());
#ifdef Q_OS_WIN
  auto isOk = processCommand(process, runCTC);
#else
  auto isOk = processShCommand(process, runCTC); //bsub <myJobs.sh
#endif

  if (isOk)
  {
      log("- Finished running ctc ");
  }
  else
  {
      log("- ctc was unsuccessful! ");
  }
  //if (wlm) delete wlm;
}

QString Controller::createCTCscenarioFolder(const QString& filePath) const
{
  QString scenarioFolderPath;
  QString ctcFileName = filePath;
  int iMaxFldrCnt = 100;

  QFileInfo info1(filePath);
  QStringList strList = info1.completeSuffix().simplified().split(".");

  if(strList.size() == 1)
    ctcFileName = filePath + ".CTC";

  QFileInfo info(ctcFileName);
  QDir baseDir(info.absoluteDir().path());
  if(info.exists())
  {
      baseDir.cdUp();
  }

  for(int i=0; i<iMaxFldrCnt; ++i)
  {
      QString fldrName = "/" + QString::number(i+1);
      scenarioFolderPath = baseDir.path().append(fldrName);
      QFileInfo newFldr(scenarioFolderPath);
      if(!newFldr.exists())
          break;
  }
  log("- Newly created Scenario folder: " + scenarioFolderPath);

  QTimer timer;
  timer.start(100);
  while(timer.remainingTime()>0)
  {
    qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
  }

  //QProcess process;
  //connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});

  QDir ctcDir(scenarioFolderPath);
  if (ctcDir.exists())//not absolutely needed 
  {
      //remove it
      ctcDir.removeRecursively();
  }
  else {
      //create it
      ctcDir.mkpath(scenarioFolderPath);
  }
  //QDir().mkdir("scenarioFolderPath");
  //processCommand(process, QString("mkdir " + scenarioFolderPath));

  return scenarioFolderPath;
}

// This function will delete the CTC scenario folder from which scenario for ALC has been created
void Controller::deleteCTCscenario(const QString& scenarioFolder)
{
    QTimer timer;
    timer.start(100);
    while(timer.remainingTime()>0)
    {
      qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    QProcess process;
    connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});


    QDir baseDir(scenarioFolder);
    if (baseDir.exists()) {
        bool isOk = baseDir.removeRecursively();

        if (isOk) {
            log("- The CTC scenario folder \"" + scenarioFolder + "\" has been successfully deleted!!!");
        }
        else {
            log("- ERROR in deleting the CTC scenario folder \"" + scenarioFolder + "\"!!!");
        }
    }
}

void Controller::createScenarioForALC(const QString& scenarioFolder)
{
    QDateTime UTC(QDateTime::currentDateTimeUtc());
    QString folderNameForALCscenario = "ALC_Scenario_" + UTC.toString("dd.MM.yyyy.hh.mm.ss");
    log("- folderNameForALCscenario: " + folderNameForALCscenario);

    QTimer timer;
    timer.start(100);
    while(timer.remainingTime()>0)
    {
      qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    QProcess process;
    connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});
    process.setWorkingDirectory(scenarioFolder);

    QDir scenarioDirec(scenarioFolder);
    QStringList fileList1 = scenarioDirec.entryList(QStringList() << "*.*" , QDir::Files);
    foreach (QString file, fileList1) {
        QFileInfo fileInfo(file);
        if(!((fileInfo.completeSuffix().contains("log")) || (fileInfo.completeSuffix().contains("project3d.CTC"))))
        {
            QDir newDir(scenarioFolder);
            newDir.mkdir(folderNameForALCscenario);
            if(fileInfo.completeSuffix().contains("project3d"))
            {
                QStringList strLst = file.split("_");
                QFile::copy(scenarioFolder + "/" + file, scenarioFolder + "/" + folderNameForALCscenario + "/" + strLst[0] + ".project3d");
            }
            else
            {
                QFile::copy(scenarioFolder + "/" + file, scenarioFolder + "/" + folderNameForALCscenario + "/" + file);
            }

        }
    }
    
    QDir cpyScenarioDirec(scenarioFolder + "/" + folderNameForALCscenario);

    auto cwd = cpyScenarioDirec.path();
    process.setWorkingDirectory(cwd);
    
    QStringList fileList2 = cpyScenarioDirec.entryList(QStringList() << "*.*" , QDir::Files);
    foreach (QString file, fileList2) {
        processCommand(process, QString("zip -u " + folderNameForALCscenario + ".zip " + file));
    }

#ifdef Q_OS_WIN
    bool isOk = false;
    // ------------- THIS PART IS BROKEN --------------//
    QStringList fileList3 = cpyScenarioDirec.entryList(QStringList() << "*.zip", QDir::Files);

	for (int i = 0; i < fileList3.count(); i++)
	{
        auto origPath = cwd + "/" + folderNameForALCscenario + ".zip ";
        QFile myDir(origPath);
        if (myDir.exists()) {
            auto renamed = myDir.rename(origPath, folderNameForALCscenario + ".zip ");

            if (!renamed)
            {
                qDebug() << "Can not rename!";
            }
        }
	}
#else
   bool isOk = processCommand(process, QString("mv " + folderNameForALCscenario + ".zip ../../."));
#endif
   if (isOk)
   {

       log("- Scenario for ALC workflow \"" + folderNameForALCscenario + ".zip\" successfully created!!!");
   }
   else {
       log("- Scenario for ALC workflow \"" + folderNameForALCscenario + ".zip\" not created, something went wrong!");
   }
}

void Controller::launchCauldronMapsTool(const QString& filePath)
{
    log("- Cauldronmaps tool to view CTC output maps");

    QFileInfo info(filePath);

    QTimer timer;
    timer.start(100);
    while(timer.remainingTime()>0)
    {
      qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    QProcess process;
    connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});
    process.setWorkingDirectory(info.absoluteDir().path());

    processCommand(process, QString("/apps/sssdev/share/cauldronmaps"));
}


void Controller::mapOutputCtcScript(const QString& filePath) const
{

  QFileInfo info(filePath);

  auto cwd = info.absoluteDir().path();


  QStringList strList = info.fileName().simplified().split(".");

  if(QFileInfo(cwd + "/" + strList[0] + "_ctc_out.project3d").exists())
  {
      QTimer timer;
      timer.start(100);
      while(timer.remainingTime()>0)
      {
        qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
      }
      QString ctcDirName = strList[0] + "_ctc_out_CauldronOutputDir";
      QDir sourceDir (cwd + "/"  + strList[0] + "_CauldronOutputDir");
	  QDir destDir(cwd + "/" + ctcDirName);
	  if (!destDir.exists()) {
          makeDirSymLinks(sourceDir, destDir);
	  }
      else {
          destDir.removeRecursively();
          QString name = destDir.absolutePath();
          makeDirSymLinks(sourceDir, destDir);
      }
      log("- CTC Scenario: " + info.absoluteDir().path());
      log("- Successfully Completed!!!");
      log("- To visualize CTC output use the button: View CTC Output Maps");
      log("- And select project3d file: \"" + strList[0] + "_ctc_out.project3d\" from \"" + info.absoluteDir().path() + "\" to view CTC output maps");
  }
  else
      log("- CTC run not Successfully Completed!!!");

}

bool Controller::processCommand(QProcess& process, const QString& command) const
{
    log("- Executing: " + command);
    process.start(command);
    if (!process.waitForStarted()) //default wait time 30 sec
    {
        process.kill();
        log("- Process stopped, did not start within 30 seconds");
        return false;
    }
    /// This is for updating the log every 10 secs
    int waitTime = 10; // sec
    while (!process.waitForFinished(waitTime))
    {
        qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    return true;
}

bool ctcWizard::Controller::processShCommand(QProcess& process, const QString& command) const
{
    process.start("sh", QStringList() << "-c" << command);
	if (!process.waitForStarted()) //default wait time 30 sec
	{
		process.kill();
		log("- Process stopped, did not start within 30 seconds");
        return false;
	}
    /// This is for updating the log every 10 secs
	int waitTime = 10; // sec
	while (!process.waitForFinished(waitTime))
	{
		qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
	}

    return true;
}

bool ctcWizard::Controller::makeDirSymLinks(const QDir& src, const QDir& desti) const
{
#ifdef Q_OS_UNIX
	QFile::link(src.absolutePath(), desti.absolutePath());
#endif

#ifdef Q_OS_WIN
	QFile::link(src.absolutePath(), desti.absolutePath().append(".lnk"));
#endif
    return false;
}

void Controller::log(const QString& text) const
{
  ui_.lineEditLog()->append(text);
}

void Controller::slotPushClearLogClicked() const
{
  ui_.lineEditLog()->setText("");
}

} // namespace ctcWizard
