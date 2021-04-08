#include "ctcController.h"

#include "controller.h"
#include "lithosphereParameterController.h"
#include "riftingHistoryController.h"
#include "model/ctcScenario.h"
#include "view/ctcTab.h"
#include "view/mainWindow.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QTableWidgetItem>
#include <QVector>
#include <QTextStream>
#include <QTabWidget>
#include <QApplication>


namespace ctcWizard
{
	const QString theDefaults::InitialCrustThickness = "35000.0";
	const QString     theDefaults::InitialMantleThickness = "90000.0";
	const QString     theDefaults::SmoothingRadius = "0";
	const QString     theDefaults::RDA = "0.0";
	const QString     theDefaults::BasaltThickness = "7000.0";

CTCcontroller::CTCcontroller(CTCtab* ctcTab,
                             CtcScenario& ctcScenario,
                             Controller* mainController) :
    QObject(mainController),
    ctcTab_(ctcTab),
    ctcScenario_(ctcScenario),
    mainController_(mainController),
    subControllers_()
{
  subControllers_.append(new LithosphereParameterController(ctcTab_->lithosphereParameterTable(), ctcScenario_, this));
  subControllers_.append(new RiftingHistoryController(ctcTab_->riftingHistoryTable(), ctcScenario_, this));

  connect(ctcTab_->pushButtonCTCrunCTC(),  SIGNAL(clicked()),                          this, SLOT(slotPushButtonCTCrunCtcClicked()));
  connect(ctcTab_->pushSelectProject3D(),   SIGNAL(clicked()),                          this, SLOT(slotPushSelectProject3dClicked()));
  connect(ctcTab_->lineEditProject3D(),     SIGNAL(textChanged(const QString&)),        this, SLOT(slotLineEditProject3dTextChanged(const QString&)));
  connect(ctcTab_->pushButtonRunFastCauldron(),   SIGNAL(clicked()),                          this, SLOT(slotpushButtonRunFastCauldronClicked()));
  connect(ctcTab_->lineEditNumProc(),     SIGNAL(textChanged(const QString&)),        this, SLOT(slotlineEditNumProcTextChanged(const QString&)));
  connect(ctcTab_->fastcldrnRunModeComboBox(),     SIGNAL(currentIndexChanged(QString)),        this, SLOT(slotRunModeComboBoxChanged(const QString&)));
  connect(ctcTab_->pushButtonCTCoutputMaps(),   SIGNAL(clicked()),                          this, SLOT(slotpushButtonCTCoutputMapsClicked()));
  connect(ctcTab_->pushButtonExportCTCoutputMaps(),   SIGNAL(clicked()),                    this, SLOT(slotpushButtonExportCTCoutputMapsClicked()));

}

QString CTCcontroller::createProject3dwithCTCUIinputs(const QString& scenarioFolderPath)
{

    QString oldProject3dFile = ctcTab_->lineEditProject3D()->text();
    QFileInfo info1(oldProject3dFile);
    QStringList strList = info1.fileName().simplified().split("/");
    QString newProject3dFile = scenarioFolderPath + "/" + strList[0] + ".CTC";
    //mainController_->log("- New Project3d file created with CTC inputs under the newly created scenario folder");

    QFile oldFile(oldProject3dFile);
    QFile newFile(newProject3dFile);

    QFileInfo info(newProject3dFile);
    if(info.exists()) newFile.remove();

    oldFile.open(QIODevice::ReadOnly | QIODevice::Text);
    newFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream oldStream(&oldFile);
    QTextStream newStream(&newFile);
    QString line = oldStream.readLine();
    newStream << line << endl;

    while(!line.isNull()){
        line = oldStream.readLine();
        newStream << line << endl;

//        //[GridMapIoTbl]
        if(line.contains("[GridMapIoTbl]",Qt::CaseSensitive)){
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;

            line = oldStream.readLine();
            //newStream << line << endl;
            while(!line.contains("[End]",Qt::CaseSensitive)){
                newStream << line << endl;
                line = oldStream.readLine();
            }
            int iCnt = 14301;
            foreach (QString RDA_Map, ctcScenario_.riftingHistoryRDAMaps_)
            {
                if((iCnt == 0) || (RDA_Map ==  "Inputs.HDF") || (RDA_Map ==  "Input.HDF") || (RDA_Map ==  " "))
                {
                    // do nothing
                }
                else
                {
                    newStream << "\"" << "CTCRiftingHistoryIoTbl" << "\"" << " "
                              << "\"" << "MAP-" + QString::number(iCnt) << "\"" << " "
                              << "\"" << "HDF5" << "\"" << " "
                              << "\"" << RDA_Map << "\"" << " "
                              //<< QString::number(iCnt)
                              << "0"
                              << endl;
                }
                iCnt++;
            }

            newStream << line << endl;
        }



        //[CTCIoTbl]
        if(line.contains("[CTCIoTbl]",Qt::CaseSensitive)){
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;

            line = oldStream.readLine();
            while(!line.contains("[End]",Qt::CaseSensitive)){
                line = oldStream.readLine();
            }
            //insert CTCIo inputs
            newStream << " \"\" " << "\"\" " << "\"\" "
                      << ctcScenario_.lithosphereParameters()[0].value << " \"\" " << ctcScenario_.lithosphereParameters()[1].value
                      << " \"\" " << "\"\" " << "\"\" " << "\"\" " << "\"\" "
                      << ctcScenario_.lithosphereParameters()[2].value << " 0.5 0.5" << endl;

            newStream << line << endl;

            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            while(!line.contains("[End]",Qt::CaseSensitive)){
                line = oldStream.readLine();
            }
            //insert CTCRiftingHistoryIoTbl inputs
            int iRow=0;
            for (const RiftingHistory& riftingHistory : ctcScenario_.riftingHistory())
            {
                QString RDAMap = "";
                QString RDAScalar = ctcScenario_.riftingHistory()[iRow].RDA;
                int iCnt = 14301;
                foreach (QString RDA_Map, ctcScenario_.riftingHistoryRDAMaps_)
                {
                    if(RDA_Map ==  ctcScenario_.riftingHistory()[iRow].RDA_Map)
                    {
                        RDAMap = "MAP-" + QString::number(iCnt);
                        //RDAScalar = "-9999";
                        break;
                    }
                    iCnt++;
                }
                QString BasaltMap = "";
                QString BasaltScalar = ctcScenario_.riftingHistory()[iRow].Basalt_Thickness;
                iCnt = 14301;
                foreach (QString Basalt_Thck_Map, ctcScenario_.riftingHistoryBasaltMaps_)
                {
                    if(Basalt_Thck_Map ==  ctcScenario_.riftingHistory()[iRow].Basalt_Thickness_Map)
                    {
                        BasaltMap = "MAP-" + QString::number(iCnt);
                        //BasaltScalar = "-9999";
                        break;
                    }
                    iCnt++;
                }

                newStream << QString::number(ctcScenario_.riftingHistory()[iRow].Age.toDouble()).rightJustified(6,' ') << " \"" << ctcScenario_.riftingHistory()[iRow].TectonicFlag
                          << "\" " << RDAScalar << " \"" << RDAMap
                          << "\" " << BasaltScalar << " \"" << BasaltMap << "\"" << endl;

              iRow++;
            }
            newStream << line << endl;

        }

    };

    oldFile.close();
    newFile.close();

    return newProject3dFile;
}

void CTCcontroller::updateProjectTxtFile(const QString& scenarioFolderPath)
{

    QString oldProjectTxtFile = scenarioFolderPath + "/Project.txt";
    QString newProjectTxtFile = scenarioFolderPath + "/Project.txt_1";
    //mainController_->log("- New Project3d file created with CTC inputs under the newly created scenario folder");

    QFile oldFile(oldProjectTxtFile);
    QFile newFile(newProjectTxtFile);

    QFileInfo info(newProjectTxtFile);
    if(info.exists()) newFile.remove();

    oldFile.open(QIODevice::ReadWrite | QIODevice::Text);
    newFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream oldStream(&oldFile);
    QTextStream newStream(&newFile);
    QString line = oldStream.readLine();
    newStream << line << endl;

    while(!line.isNull()){
        line = oldStream.readLine();
        //newStream << line << endl;

        //Scenario
        QStringList strList = line.simplified().split(":");
        if(strList[0].contains("Scenario",Qt::CaseSensitive)){
            newStream << "Scenario: " << strList[1] << " CTC " << endl;
        }

        //Top Boundary Conditions
        else if (strList[0].contains("Top Boundary Conditions",Qt::CaseSensitive)){
            newStream << "Top Boundary Conditions: Updated by CTC Outputs " << endl;
        }

        //Bottom Boundary Conditions
        else if(strList[0].contains("Bottom Boundary Conditions",Qt::CaseSensitive)){
            newStream << "Bottom Boundary Conditions: Updated by CTC Outputs " << endl;
        }
        else
            newStream << line << endl;

    };

    oldFile.close();
    newFile.close();

    oldFile.remove();
    newFile.rename(oldProjectTxtFile);

}

void GetTableFromProject3d(QString& fileName, QVector<QString>& TblVector, QString& tblName)
{
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream stream(&file);
    QString line = stream.readLine();
    while(!line.isNull()){
        line = stream.readLine();
        if(line.contains(tblName,Qt::CaseSensitive)){
            //skip 3 lines
            line = stream.readLine();
            line = stream.readLine();
            line = stream.readLine();

            line = stream.readLine();
            while(!line.contains("[End]",Qt::CaseSensitive)){
                TblVector << line;
                line = stream.readLine();
            }
            line = stream.readLine();
            break;
        }

    };
    file.close();
}

void CTCcontroller::upateProject3dfileToStoreOutputs(const QString& project3dFilePath)
{
    QFile oldFile(project3dFilePath);
    QFileInfo info1(project3dFilePath);

    QStringList strList = info1.fileName().simplified().split("/");
    QString newProject3dFile = info1.absolutePath() + "/" + strList[0] + "_1";
    QFile newFile(newProject3dFile);

    newFile.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream newStream(&newFile);


    oldFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream oldStream(&oldFile);
    QString line = oldStream.readLine();
    newStream << line << endl;

    while(!line.isNull()){
        line = oldStream.readLine();
        newStream << line << endl;

        //[FilterTimeIoTbl]
        if(line.contains("[FilterTimeIoTbl]",Qt::CaseSensitive)){
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;

            line = oldStream.readLine();
            newStream << line << endl;

            newStream << "LithoStaticPressure" << " " << "1d" << " " << "SedimentsOnly" << " " << "Simple" << endl;
            newStream << "LithoStaticPressure" << " " << "3d" << " " << "SedimentsPlusBasement" << " " << "Simple" << endl;

            while(!line.contains("[End]",Qt::CaseSensitive)){
                line = oldStream.readLine();
                QStringList strLst = line.simplified().split(" ");
                if((strLst[0].contains("LithoStaticPressure") && strLst[1].contains("1d")) ||
                    (strLst[0].contains("LithoStaticPressure") && strLst[1].contains("3d")))
                {
                    //
                }
                else
                    newStream << line << endl;
            }
        }

        /*//[OutputTablesFileIoTbl]
        if(line.contains("[OutputTablesFileIoTbl]",Qt::CaseSensitive)){
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            line = oldStream.readLine();
            newStream << line << endl;
            newStream << "[End]" << endl;

            newStream << ";" << endl;
            newStream << ";" << endl;
            newStream << "; OutputTablesIoTbl" << endl;
            newStream << ";" << endl;
            newStream << "[OutputTablesIoTbl]" << endl;
            newStream << ";v100" << endl;
            newStream << "TableName" << endl;
            newStream << "  ()" << endl;
            newStream << "[End]" << endl;
            newStream << ";" << endl;
            break;
        } */

        //OutputTablesIoTbl
        if(line.contains("[OutputTablesIoTbl]",Qt::CaseSensitive)){
            line = oldStream.readLine();
            while(!line.contains("[End]",Qt::CaseSensitive)){
                if(line.contains("\"TimeIoTbl\"",Qt::CaseSensitive)){
                    newStream << "#\"TimeIoTbl\"" << endl;
                    line = oldStream.readLine();
                }
                else
                {
                    newStream << line << endl;
                    line = oldStream.readLine();
                }
            }
            newStream << line << endl;
        }

    };

    oldFile.close();
    newFile.close();

    // rename newly created file to old file name
    bool boo = oldFile.remove();
    newFile.rename(project3dFilePath);
}


bool CTCcontroller::validateCTCinputFields()
{

    bool isValidFieldValues = true;
    bool validate;
    double value;

    //[CTCIoTbl]
    value = ctcScenario_.lithosphereParameters()[0].value.toDouble(&validate);
    if(!validate)
    {
        mainController_->log("- Invalid Input for: Initial Crust Thickness");
        isValidFieldValues = false;
    }

    value = ctcScenario_.lithosphereParameters()[1].value.toDouble(&validate);
    if(!validate)
    {
        mainController_->log("- Invalid Input for: Initial Mantle Thickness");
        isValidFieldValues = false;
    }
    // Smoothing Radius is actually the no. of cells to consider for map smoothing
    value = ctcScenario_.lithosphereParameters()[2].value.toInt(&validate);
    if(!validate)
    {
        mainController_->log("- Invalid Input for: Smoothing Radius");
        isValidFieldValues = false;
    }

    //[CTCRiftingHistoryIoTbl]
    int iRow=0;
    for (const RiftingHistory& riftingHistory : ctcScenario_.riftingHistory())
    {
        value = ctcScenario_.riftingHistory()[iRow].RDA.toDouble(&validate);
        if(!validate)
        {
            mainController_->log("- Invalid Input for RDA Adjustment in Rifting History Table; for Age: " + ctcScenario_.riftingHistory()[iRow].Age);
            isValidFieldValues = false;
        }
        value = ctcScenario_.riftingHistory()[iRow].Basalt_Thickness.toDouble(&validate);
        if(!validate)
        {
            mainController_->log("- Invalid Input for Max Basalt Thickness in Rifting History Table; for Age: " + ctcScenario_.riftingHistory()[iRow].Age);
            isValidFieldValues = false;
        }
        iRow++;
    }

    return isValidFieldValues;
}

void CTCcontroller::slotpushButtonCTCoutputMapsClicked()
{
    //
    mainController_->launchCauldronMapsTool(ctcScenario_.project3dPath());
}

void CTCcontroller::slotPushButtonCTCrunCtcClicked()
{
    QString fastcldrnRunMode;
    if(ctcScenario_.project3dPath().isEmpty())
        mainController_->log("- Please select project3d file");
    else
    {
        bool isValidFieldValues = validateCTCinputFields();
        if(ctcScenario_.runMode().isEmpty())
            fastcldrnRunMode = "Decompaction";
        else
            fastcldrnRunMode = ctcScenario_.runMode();
        if(isValidFieldValues)
        {
            bool isResultsAvailable = isFastCauldronResultsAvailable(fastcldrnRunMode);
            if(!isResultsAvailable)
            {
                mainController_->log("- Cauldron results not exist");
                mainController_->log("- !!!Run Cauldron");
            }
            else
            {
                QString scenarioFolderPath = mainController_->createCTCscenarioFolder(ctcScenario_.project3dPath());
                QString ctcFilenameWithPath = createProject3dwithCTCUIinputs(scenarioFolderPath);
                mainController_->executeCtcScript(ctcFilenameWithPath, ctcScenario_.numProc());
                mainController_->mapOutputCtcScript(ctcFilenameWithPath);
                updateProjectTxtFile(scenarioFolderPath);
            }
        }
    }

}

bool CTCcontroller::isFastCauldronResultsAvailable(const QString& fastcldrnRunMode) const
{

    QString project3dFile = ctcTab_->lineEditProject3D()->text();
    QFileInfo info(project3dFile);
    QStringList strLst = info.fileName().simplified().split(".");
    QDir dir(info.dir().absolutePath() + "/" + strLst[0] + "_CauldronOutputDir");
    if(!dir.exists()) return false;

    //collect fastcauldron output files from project3d file and check whether these files exist in output folder
    // SnapshotIoTbl Table
    QVector<QString> SnapshotIoTblVector;
    QString tblName = "[SnapshotIoTbl]";
    GetTableFromProject3d(project3dFile, SnapshotIoTblVector, tblName);

    for(int i = 0; i < SnapshotIoTblVector.size(); ++i)
    {
        QStringList strLst1 = SnapshotIoTblVector[i].simplified().split(" ");
        QStringList fileLst = strLst1[strLst1.size()-1].simplified().split("\"");
        QFile resFile(dir.filePath(fileLst[1]));
        if(!resFile.exists()) return false;
    }

    if(fastcldrnRunMode.contains("Decompaction"))
    {
        QFile resFile(dir.filePath("HydrostaticDecompaction_Results.HDF"));
        if(!resFile.exists()) return false;
    }
    else if(fastcldrnRunMode.contains("Hydrostatic"))
    {
        QFile resFile(dir.filePath("HydrostaticTemperature_Results.HDF"));
        if(!resFile.exists()) return false;
    }
    else if(fastcldrnRunMode.contains("Iteratively Coupled"))
    {
        QFile resFile(dir.filePath("PressureAndTemperature_Results.HDF"));
        if(!resFile.exists()) return false;
    }

    return true;
}

void CTCcontroller::slotpushButtonRunFastCauldronClicked()
{
    //
    if(ctcScenario_.project3dPath().isEmpty())
        mainController_->log("- Please select project3d file");
    else
    {
        bool validate;
        double value;
        bool isValidFieldValues;
        int iMaxNumProc = 1000;
        QString fastcldrnRunMode;
        value = ctcScenario_.numProc().toInt(&validate);

        if(ctcScenario_.runMode().isEmpty())
            fastcldrnRunMode = "Decompaction";
        else
            fastcldrnRunMode = ctcScenario_.runMode();

        if(!validate)
        {
            mainController_->log("- Invalid Input for: Number of Processors (Cauldron)");
            isValidFieldValues = false;
        }
        else if(value > iMaxNumProc)
        {
            mainController_->log("- Invalid Input for: Number of Processors (Cauldron)");
            mainController_->log("- Number of Processors for Cauldron run should not exceed 1000");
            isValidFieldValues = false;
        }
        else {
            isValidFieldValues = true;
        }

        if(isValidFieldValues)
        {
            upateProject3dfileToStoreOutputs(ctcScenario_.project3dPath());
            bool isResultsAvailable = isFastCauldronResultsAvailable(fastcldrnRunMode);
            if(!isResultsAvailable)
            {
                mainController_->log("- Cauldron results not exist for Cauldron mode: " + fastcldrnRunMode);
                mainController_->executeFastcauldronScript(ctcScenario_.project3dPath(), fastcldrnRunMode, ctcScenario_.numProc());
            }
            else
            {
                mainController_->log("- Cauldron results already exist for " + fastcldrnRunMode);
            }
        }
    }

}

void CTCcontroller::slotpushButtonExportCTCoutputMapsClicked()
{

    QString scenarioFolder = QFileDialog::getExistingDirectory(ctcTab_,"Select CTC Scenario Folder which contains CTC output maps",
                                                               ctcScenario_.project3dPath());
    mainController_->log("");
    mainController_->log("Selected CTC Scenario: " + scenarioFolder);

    QDir scenarioDirec(scenarioFolder);
    QStringList fileList = scenarioDirec.entryList(QStringList() << "*.CTC" , QDir::Files);
    if(fileList.empty())
    {
        mainController_->log("");
        mainController_->log("- Selected Scenario folder doesn't contain CTC output maps!!!");
        mainController_->log("- Please select valid CTC Scenario folder!!!");
    }
    else
    {
        QStringList strLst = fileList[0].split(".");
#ifdef Q_OS_WIN
    QDir ctcOutputDir(scenarioFolder + "/" + strLst[0] + "_ctc_out_CauldronOutputDir" + ".lnk");
#endif
#ifdef Q_OS_LINUX
    QDir ctcOutputDir(scenarioFolder + "/" + strLst[0] + "_ctc_out_CauldronOutputDir");
#endif   
        if(!ctcOutputDir.exists())
        {
            //
            mainController_->log("");
            mainController_->log("- Selected Scenario folder doesn't contain CTC output maps!!!");
            mainController_->log("- Please select valid CTC Scenario folder!!!");
        }
        else
        {
            mainController_->log("");
            mainController_->log("This CTC Scenario is used to create Scenario for ALC Work-flow: ");
            mainController_->createScenarioForALC(scenarioFolder);
            //mainController_->deleteAllCTCscenarios(scenarioFolder);
        }
    }



}

void CTCcontroller::slotPushSelectProject3dClicked()
{
    // this was a botch up; max fileName on Lnux is 255 chars & maximum path of 4096 characters
  int iMaxPathLen = 4096;

  QString fileName = QFileDialog::getOpenFileName(ctcTab_,"Select project file",ctcScenario_.project3dPath(),
                                                    "Project files (*.project3d *.CTC)");

  if(fileName.length() > iMaxPathLen)
  {
      mainController_->log("Selected Project3d file: " + fileName);
      mainController_->log("Selected Scenario Path is too lengthy to handle!!! Shorten the folder names and path!!!");
  }
  else
  {
      ctcTab_->lineEditProject3D()->setText(fileName);

      ctcScenario_.clearLithosphereTable();
      ctcScenario_.clearRiftingHistoryTable();
      LithosphereParameterController(ctcTab_->lithosphereParameterTable(), ctcScenario_, mainController_);
      RiftingHistoryController(ctcTab_->riftingHistoryTable(), ctcScenario_, mainController_);

      //[CTCIoTbl]
      QVector<QString> lithosphereTblVector;
      QString tblName = "[CTCIoTbl]";
      GetTableFromProject3d(fileName, lithosphereTblVector, tblName);


      if(lithosphereTblVector.size())
      {
          QString &str = lithosphereTblVector[0];
          QStringList strLst = str.simplified().split(" ");
          QString strHcuIni, strHLMuIni, strFilterHalfWidth, strUpLoContCrRatio, strUpLoOceCrRatio;
          if(strLst[1].contains("\""))
          {
              strHcuIni = strLst[3];
              strHLMuIni = strLst[5];
              strFilterHalfWidth =strLst[11];
          }
          else
          {
              strHcuIni = strLst[3];
              strHLMuIni = strLst[5];
              strFilterHalfWidth =strLst[11];
          }
          ctcScenario_.addLithosphereParameter("Initial Crust Thickness [m]",strHcuIni);
          ctcScenario_.addLithosphereParameter("Initial Mantle Thickness [m]",strHLMuIni);
          ctcScenario_.addLithosphereParameter("Smoothing Radius [number of cells]",strFilterHalfWidth);
      }
      else
      {
          ctcScenario_.addLithosphereParameter("Initial Crust Thickness [m]", theDefaults::InitialCrustThickness);
          ctcScenario_.addLithosphereParameter("Initial Mantle Thickness [m]", theDefaults::InitialMantleThickness);
          ctcScenario_.addLithosphereParameter("Smoothing Radius [number of cells]", theDefaults::SmoothingRadius);
      }

      // Rifting History Table
      QVector<QString> riftHistTblVector;
      tblName = "[CTCRiftingHistoryIoTbl]";
      GetTableFromProject3d(fileName, riftHistTblVector, tblName);

      // SurfaceDepthIoTbl
      QVector<QString> SurfaceDepthIoTblVector;
      QString strHasPWD;
      tblName = "[SurfaceDepthIoTbl]";
      QVector<QString> strVectHasPWD;
      GetTableFromProject3d(fileName, SurfaceDepthIoTblVector, tblName);
      if(SurfaceDepthIoTblVector.size())
      {
          for(int i = 0; i < SurfaceDepthIoTblVector.size(); ++i)
          {
              QStringList strLst = SurfaceDepthIoTblVector[i].simplified().split(" ");
              strVectHasPWD << strLst[0];
          }
      }

      if(riftHistTblVector.size())
      {
          for(int i = 0; i < riftHistTblVector.size(); ++i)
          {
              //QString &str = riftHistTblVector[i];
              strHasPWD = "N";
              QStringList strLst = riftHistTblVector[i].simplified().split(" ");
              QString strTectonicFlg, strRDAMap, strBasaltMap;
              if(strLst[1].contains("Flexural"))
                  strTectonicFlg = "Flexural Basin";
              else if(strLst[1].contains("Active"))
                  strTectonicFlg = "Active Rifting";
              else if(strLst[1].contains("Passive"))
                  strTectonicFlg = "Passive Margin";

              int len = strLst[6].size();
              if(strLst[4].size() == 2)
                  strRDAMap = "";
              else
                  strRDAMap = strLst[4].mid(1,len-2);
              if(strLst[6].size() == 2)
                  strBasaltMap = "";
              else
                  strBasaltMap = strLst[6].mid(1,len-2);

              for(int j = 0; j < strVectHasPWD.size(); ++j)
              {
                  if(strVectHasPWD[j].contains(strLst[0]))
                  {
                      strHasPWD = "Y";
                      break;
                  }
              }
              ctcScenario_.addRiftingHistory(strLst[0], strHasPWD, strTectonicFlg, strLst[3], strRDAMap, strLst[5], strBasaltMap);
          }
      }
      else
      {
          // StratIoTbl
          QVector<QString> stratIoTblVector;
          tblName = "[StratIoTbl]";
          GetTableFromProject3d(fileName, stratIoTblVector, tblName);
          for(int i = 0; i < stratIoTblVector.size(); ++i)
          {
              QString &str = stratIoTblVector[i];
              QStringList strLst1 = str.simplified().split(" ");
              QString strAge;
              strHasPWD = "N";
              if(strLst1[0].endsWith("\""))
                  strAge = strLst1[1];
              else
              {
                  for(int j = 1; j < strLst1.size(); ++j)
                  {
                      if(strLst1[j].endsWith("\""))
                      {
                          strAge = strLst1[j+1];
                          break;
                      }
                  }
              }

              for(int j = 0; j < strVectHasPWD.size(); ++j)
              {
                  if(strVectHasPWD[j].contains(strAge))
                  {
                      strHasPWD = "Y";
                      break;
                  }

              }

              if(i == 0)
                  ctcScenario_.addRiftingHistory(strAge, strHasPWD, "Flexural Basin",theDefaults::RDA,"",theDefaults::BasaltThickness,"");
              else if(i == stratIoTblVector.size() - 1)
                  ctcScenario_.addRiftingHistory(strAge, strHasPWD, "Active Rifting", theDefaults::RDA, "", theDefaults::BasaltThickness, "");
              else
                  ctcScenario_.addRiftingHistory(strAge, strHasPWD, "Passive Margin", theDefaults::RDA, "", theDefaults::BasaltThickness, "");
          }
      }

      QFileInfo info(ctcScenario_.project3dPath());
      QDir dir(info.absoluteDir().path());
      QStringList filters;
      filters << "*.HDF";

      ctcScenario_.riftingHistoryBasaltMaps_ = QStringList() << " ";
      ctcScenario_.riftingHistoryRDAMaps_ = QStringList() << " ";

      foreach (QString file, dir.entryList(filters, QDir::Files))
      {
          ctcScenario_.riftingHistoryRDAMaps_ << file;
          ctcScenario_.riftingHistoryBasaltMaps_ << file;
      }


      LithosphereParameterController(ctcTab_->lithosphereParameterTable(), ctcScenario_, mainController_);
      RiftingHistoryController(ctcTab_->riftingHistoryTable(), ctcScenario_, mainController_);

      mainController_->log("Selected Scenario: \"" + info.absoluteDir().path() + "\"");
      mainController_->log("Loaded project3d file in CTC-UI: \"" + info.fileName() + "\"");
      mainController_->log("- If RDA Adjustment Maps or Basalt Thickness Maps are selected, then respective scalar values are not considered !!!");
  }

}


void CTCcontroller::slotLineEditProject3dTextChanged(const QString& project3dPath)
{
  ctcScenario_.setProject3dPath(project3dPath);
}

void CTCcontroller::slotlineEditNumProcTextChanged(const QString& numProc)
{
  ctcScenario_.setnumProc(numProc);
}


void CTCcontroller::slotRunModeComboBoxChanged(const QString& runMode)
{
  ctcScenario_.setrunMode(runMode);
}

} // namespace ctcWizard
