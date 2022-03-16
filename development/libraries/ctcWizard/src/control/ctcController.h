#pragma once
#include <map>
#include <QObject>
#include <QVector>


#include "cmbAPI.h"


class QTextStream;
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

	struct theProject3dEntryNames
	{
		// GridMapIoTbl
		static const char* s_GridMapIoTblName;
		static const char* s_GridMapIoMapReferredByColName;
		static const char* s_GridMapIoMapNameColName;
		static const char* s_GridMapIoMapTypeColName;
		static const char* s_GridMapIoMapFileNameColName;
		static const char* s_GridMapIoMapSeqNbrColName;
		//ContCrustalThicknessIoTbl
		static const char* s_ContCrustalThicknessIoTblName;
		static const char* s_ContCrustalThicknessIoAgeColName;
		static const char* s_ContCrustalThicknessIoThicknessColName;
		static const char* s_ContCrustalThicknessIoThicknessGridColName;
		//OceaCrustalThicknessIoTbl
		static const char* s_OceaCrustalThicknessIoTblName;
		static const char* s_OceaCrustalThicknessIoAgeColName;
		static const char* s_OceaCrustalThicknessIoThicknessColName;
		static const char* s_OceaCrustalThicknessIoThicknessGridColName;
		//CTCIoTbl
		static const char* s_CTCIoTblName;
		static const char* s_CTCIoSurfaceNameColName;
		static const char* s_CTCIoTRIniColName;
		static const char* s_CTCIoTRIniGridColName;
		static const char* s_CTCIoHCuIniColName;
		static const char* s_CTCIoHCuIniGridColName;
		static const char* s_CTCIoHLMuIniColName;
		static const char* s_CTCIoHLMuIniGridColName;
		static const char* s_CTCIoDeltaSLColName;
		static const char* s_CTCIoDeltaSLGridColName;
		static const char* s_CTCIoHBuColName;
		static const char* s_CTCIoHBuGridColName;
		static const char* s_CTCIoFilterHalfWidthColName;
		static const char* s_CTCIoUpperLowerContinentalCrustRatioColName;
		static const char* s_CTCIoUpperLowerOceanicCrustRatioColName;
		//StratIoTbl
		static const char* s_StratIoTblName;
		static const char* s_StratIoDepoAgeColName;
		//CTCRiftingHistoryIoTbl
		static const char* s_CTCRiftingHistoryIoTblName;
		static const char* s_CTCRiftingHistoryIoAgeColName;
		static const char* s_CTCRiftingHistoryIoTectonicFlagColName;
		static const char* s_CTCRiftingHistoryIoDeltaSLColName;
		static const char* s_CTCRiftingHistoryIoDeltaSLGridColName;
		static const char* s_CTCRiftingHistoryIoHBuColName;
		static const char* s_CTCRiftingHistoryIoHBuGridColName;
		//SurfaceDepthIoTbl
		static const char* s_SurfaceDepthIoTblName;
		static const char* s_SurfaceDepthIoAgeColName;
		// 
		// TO DO
		// list out all the table names and the column entries used in here

	};

	class CTCcontroller : public QObject
	{
		Q_OBJECT

	public:
		CTCcontroller(CTCtab* ctcTab,
			CtcScenario& ctcScenario,
			Controller* mainController);

		// load project
		mbapi::Model cldrnProject;

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
		void CreateRifthingHistoryBasaltMapUsedList(std::vector<QString>& BasaltMapsUsed) const;
		void CreateRifthingHistoryRDAMapList(std::vector<QString>& RDAMapsUsed) const;
		void updateProjectTxtFile(const QString& scenarioFolderPath) const;
		/// <summary>
		/// this checks for the FastCAuldron result type
		/// </summary>
		/// <param name="fastcldrnRunMode"></param>
		/// the mode
		/// <returns></returns>
		/// 1 => de-compaction not available
		/// 2 => HydroStatic not available
		/// 3 => It_coupled not available
		int isFastCauldronResultsAvailable(const QString* fastcldrnRunMode) const;
		static std::map<int, QString> PtModeMap;
		// TO DO: UI validation can me incorporated  here
		bool validateCTCinputFields()const;
		mbapi::Model::ReturnCode upateProject3dfileToStoreOutputs(const QString& filePath);

		CTCtab* ctcTab_;
		CtcScenario& ctcScenario_;
		Controller* mainController_;
		QVector<QObject*> subControllers_;
		inline int findLastMapSqNumber();
		inline void setP3FileNameForCTCScenarios(QString name);
		inline QString getP3FileNameForCTCScenarios(void) const;
		QString m_CTCP3FileName;
	};

} // namespace ctcWizard


