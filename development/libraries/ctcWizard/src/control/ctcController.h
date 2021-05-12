#pragma once

#include <QObject>
#include <QVector>
class QTextStream;
#include <map>
class QString;
class QTableWidgetItem;

namespace ctcWizard
{

class CtcScenario;
class Controller;
class CTCtab;

struct theDefaults
{
    static const QString    InitialCrustThickness;
    static const QString    InitialMantleThickness;
    static const QString    SmoothingRadius;
    static const QString    RDA;
    static const QString    BasaltThickness;
};

class CTCcontroller : public QObject
{
  Q_OBJECT

public:
    CTCcontroller(CTCtab* ctcTab,
                CtcScenario& ctcScenario,
                Controller* mainController);

private slots:
    void slotPushSelectProject3dClicked();
    void slotPushButtonCTCrunCtcClicked();
    void slotLineEditProject3dTextChanged(const QString& project3dPath);
    void slotpushButtonRunFastCauldronClicked();
    void slotlineEditNumProcTextChanged(const QString& numProc);
    void slotRunModeComboBoxChanged(const QString& runMode);
    void slotpushButtonCTCoutputMapsClicked();
    void slotpushButtonExportCTCoutputMapsClicked();

private:
    QString createProject3dwithCTCUIinputs(const QString& scenarioFolderPath);
    void CreateRifthingHistoryBasaltMapUsedList(std::vector<QString>& BasaltMapsUsed);
    void CreateRifthingHistoryRDAMapList(std::vector<QString>& RDAMapsUsed);
    void updateProjectTxtFile(const QString& scenarioFolderPath);
    /// <summary>
    /// this checks for the FastCAuldron result type
    /// </summary>
    /// <param name="fastcldrnRunMode"></param>
    /// the mode
    /// <returns></returns>
    /// 1 => de-compaction not available
    /// 2 => HydroStatic not available
    /// 3 => It_coupled not available
    int isFastCauldronResultsAvailable(const QString* fastcldrnRunMode=nullptr) const;
    static std::map<int, QString> PtModeMap;
    bool validateCTCinputFields();
    void upateProject3dfileToStoreOutputs(const QString& filePath);

    CTCtab* ctcTab_;
    CtcScenario& ctcScenario_;
    Controller* mainController_;
    QVector<QObject*> subControllers_;
    int findLastMapSqNumber(const QString& scenarioFolderPath)const;

};

} // namespace ctcWizard
