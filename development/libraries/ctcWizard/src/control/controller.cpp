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

namespace ctcWizard
{

Controller::Controller() : QObject(),
  ui_(),
  ctcScenario_(),
  subControllers_()
{
  createConnections();

  subControllers_.append(new CTCcontroller(ui_.ctcTab(), ctcScenario_, this));

  ui_.show();
}

void Controller::createConnections() const
{
  connect(ui_.actionExit(),     SIGNAL(triggered()), this, SLOT(slotActionExitTriggered()));
  connect(ui_.pushClearLog(),   SIGNAL(clicked()),   this, SLOT(slotPushClearLogClicked()));
}

void Controller::slotActionExitTriggered()
{
  ui_.close();
}

void Controller::slotActionOpenFileTriggered()
{
  ctcScenario_.setCtcFilePathCTC(QFileDialog::getOpenFileName(ui_.centralWidget(),
                                                                "Load CTC file",
                                                                ctcScenario_.ctcFilePathCTC(),
                                                                "CTC (*.ctc)"));

  QFile file(ctcScenario_.ctcFilePathCTC());
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    log("Failed loading CTC file");
    return;
  }

  QTextStream in(&file);
  QString logText;
  while(!in.atEnd())
  {
    logText.append(in.readLine() + "\n");
  }

  log(logText);
}

void Controller::executeFastcauldronScript(const QString& filePath, const QString& fastcldrnRunMode, const QString& numProc) const
{
  log("- Start running fastcauldron");
  QTimer timer;
  timer.start(100);
  while(timer.remainingTime()>0)
  {
    qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
  }

  QString cldrnRunMode;
  if(fastcldrnRunMode.contains("Decompaction"))
  {
      cldrnRunMode = "-decompaction";
  }
  else if(fastcldrnRunMode.contains("Hydrostatic"))
  {
      cldrnRunMode = "-temperature";
  }
  else if(fastcldrnRunMode.contains("Thermal/Pressure"))
  {
      cldrnRunMode = "-itcoupled";
  }

  QProcess process;
  connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});

  QFileInfo info(filePath);
  process.setWorkingDirectory(info.absoluteDir().path());
  //
  QStringList strLst = info.fileName().simplified().split(".");
  QDir dir(info.dir().absolutePath() + "/" + strLst[0] + "_CauldronOutputDir");
  if(dir.exists()) processCommand(process, QString("rm -rf "+ strLst[0] + "_CauldronOutputDir"));
  //v2020.01nightly --- This should be changed
  processCommand(process, QString("mpirun -n " + numProc + " /apps/sssdev/ibs/v2020.01nightly/LinuxRHEL64/bin/fastcauldron -project "
                                  + filePath + " " + cldrnRunMode + " -allproperties"));
  log("- Finished running fastcauldron");
}

void Controller::executeCtcScript(const QString& ctcFilenameWithPath) const
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
  foreach (QString file, fileList) {
      QFileInfo fileInfo(file);
      if(!((fileInfo.completeSuffix().contains("log")) || (fileInfo.completeSuffix().contains("zip")) || (fileInfo.completeSuffix().contains("project3d"))))
      {
          QFile::copy(baseDir.path() + "/" + file, scenarioDirec.path() + "/" + file);
      }
  }
  QStringList strList = info.fileName().simplified().split(".");
  processCommand(process, QString("cp -r ../" + strList[0] + "_CauldronOutputDir " + "."));

  processCommand(process, QString("/apps/sssdev/ibs/v2020.01nightly/LinuxRHEL64/bin/fastctc -merge -project " + ctcFilenameWithPath + " -save " + info.absoluteDir().path() + "/" + strList[0] + "_ctc_out.project3d"));
  log("- Finished running ctc ");
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

  QProcess process;
  connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});

  processCommand(process, QString("mkdir " + scenarioFolderPath));

  return scenarioFolderPath;
}

void Controller::deleteAllCTCscenarios(const QString& scenarioFolder)
{
    QTimer timer;
    timer.start(100);
    int iMaxFldrCnt = 100;
    while(timer.remainingTime()>0)
    {
      qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
    }

    QProcess process;
    connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});


    QDir baseDir(scenarioFolder);
    baseDir.cdUp();

    for(int i=0; i<iMaxFldrCnt; ++i)
    {
        QString fldrName = "/" + QString::number(i+1);
        QString ctcScenarioFolderPath = baseDir.path().append(fldrName);
        QFileInfo newFldr(ctcScenarioFolderPath);
        if(newFldr.exists())
            processCommand(process, QString("rm -r " + ctcScenarioFolderPath));
        else
            break;
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

    process.setWorkingDirectory(scenarioFolder + "/" + folderNameForALCscenario);
    QDir cpyScenarioDirec(scenarioFolder + "/" + folderNameForALCscenario);
    QStringList fileList2 = cpyScenarioDirec.entryList(QStringList() << "*.*" , QDir::Files);
    foreach (QString file, fileList2) {
        processCommand(process, QString("zip -u " + folderNameForALCscenario + ".zip " + file));
    }

    processCommand(process, QString("mv " + folderNameForALCscenario + ".zip ../../."));

    log("- Scenario for ALC workflow \"" + folderNameForALCscenario + ".zip\" successfully created!!!");

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
  QStringList strList = info.fileName().simplified().split(".");

  QFileInfo info1(info.absoluteDir().path() + "/"  + strList[0] + "_ctc_out.project3d");
  if(info1.exists())
  {
      QTimer timer;
      timer.start(100);
      while(timer.remainingTime()>0)
      {
        qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
      }

      QProcess process;
      connect(&process,&QProcess::readyReadStandardOutput,[&](){log(QString(process.readAllStandardOutput()));});
      process.setWorkingDirectory(info.absoluteDir().path());


      QDir dir(info.absoluteDir().path() + "/" + strList[0] + "_ctc_out_CauldronOutputDir");
      if(dir.exists())
          processCommand(process, QString("rm -rf " + info.absoluteDir().path() + "/" + strList[0] + "_ctc_out_CauldronOutputDir"));

      processCommand(process, QString("ln -s " + strList[0] + "_CauldronOutputDir " + strList[0] + "_ctc_out_CauldronOutputDir"));
      log("- CTC Sceario: " + info.absoluteDir().path());
      log("- Successfully Completed!!!");
      log("- To visualize CTC output using the button: View CTC Output Maps");
      log("- And select project3d file: " + strList[0] + "_ctc_out.project3d from the newly created scenario folder to view CTC output maps");
  }
  else
      log("- CTC run not Successfully Completed!!!");

}

void Controller::processCommand(QProcess& process, const QString& command) const
{
  //log(command);
  process.start(command);
  if (!process.waitForStarted())
  {
    process.kill();
    log("- Process stopped, did not start within 30 seconds");
  }
  while (!process.waitForFinished())
  {
    qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
  }
}

void Controller::log(const QString& text) const
{
  ui_.lineEditLog()->append(text);
}

void Controller::slotPushClearLogClicked()
{
  ui_.lineEditLog()->setText("");
}

} // namespace ctcWizard
