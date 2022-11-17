#include "ctcController.h"

#include "controller.h"
#include "lithosphereParameterController.h"
#include "riftingHistoryController.h"
#include "model/ctcScenario.h"
#include "view/ctcTab.h"
#include "view/mainWindow.h"
#include "NumericFunctions.h"



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
#include <QDebug>
#include <QTabWidget>
#include <QApplication>
#include "Qt_Utils.h"

namespace ctcWizard
{
	const char* theProject3dEntryNames::s_GridMapIoTblName = "GridMapIoTbl";
	const char* theProject3dEntryNames::s_GridMapIoMapReferredByColName = "ReferredBy";
	const char* theProject3dEntryNames::s_GridMapIoMapNameColName = "MapName";
	const char* theProject3dEntryNames::s_GridMapIoMapTypeColName = "MapType";
	const char* theProject3dEntryNames::s_GridMapIoMapFileNameColName = "MapFileName";
	const char* theProject3dEntryNames::s_GridMapIoMapSeqNbrColName = "MapSeqNbr";

	const char* theProject3dEntryNames::s_ContCrustalThicknessIoTblName = "ContCrustalThicknessIoTbl";
	const char* theProject3dEntryNames::s_ContCrustalThicknessIoAgeColName = "Age";
	const char* theProject3dEntryNames::s_ContCrustalThicknessIoThicknessColName = "Thickness";
	const char* theProject3dEntryNames::s_ContCrustalThicknessIoThicknessGridColName = "ThicknessGrid";

	const char* theProject3dEntryNames::s_OceaCrustalThicknessIoTblName = "OceaCrustalThicknessIoTbl";
	const char* theProject3dEntryNames::s_OceaCrustalThicknessIoAgeColName = "Age";
	const char* theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessColName = "Thickness";
	const char* theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessGridColName = "ThicknessGrid";

	const char* theProject3dEntryNames::s_CTCIoTblName = "CTCIoTbl";
	const char* theProject3dEntryNames::s_CTCIoSurfaceNameColName = "SurfaceName";
	const char* theProject3dEntryNames::s_CTCIoTRIniColName = "TRIni";
	const char* theProject3dEntryNames::s_CTCIoTRIniGridColName = "TRIniGrid";
	const char* theProject3dEntryNames::s_CTCIoHCuIniColName = "HCuIni";
	const char* theProject3dEntryNames::s_CTCIoHCuIniGridColName = "HCuIniGrid";
	const char* theProject3dEntryNames::s_CTCIoHLMuIniColName = "HLMuIni";
	const char* theProject3dEntryNames::s_CTCIoHLMuIniGridColName = "HLMuIniGrid";
	const char* theProject3dEntryNames::s_CTCIoDeltaSLColName = "DeltaSL";
	const char* theProject3dEntryNames::s_CTCIoDeltaSLGridColName = "DeltaSLGrid";
	const char* theProject3dEntryNames::s_CTCIoHBuColName = "HBu";
	const char* theProject3dEntryNames::s_CTCIoHBuGridColName = "HBuGrid";
	const char* theProject3dEntryNames::s_CTCIoFilterHalfWidthColName = "FilterHalfWidth";
	const char* theProject3dEntryNames::s_CTCIoUpperLowerContinentalCrustRatioColName = "UpperLowerContinentalCrustRatio";
	const char* theProject3dEntryNames::s_CTCIoUpperLowerOceanicCrustRatioColName = "UpperLowerOceanicCrustRatio";

	const char* theProject3dEntryNames::s_StratIoTblName = "StratIoTbl";
	const char* theProject3dEntryNames::s_StratIoDepoAgeColName = "DepoAge";

	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoTblName = "CTCRiftingHistoryIoTbl";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoAgeColName = "Age";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoTectonicFlagColName = "TectonicFlag";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLColName = "DeltaSL";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLGridColName = "DeltaSLGrid";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoHBuColName = "HBu";
	const char* theProject3dEntryNames::s_CTCRiftingHistoryIoHBuGridColName = "HBuGrid";

	const char* theProject3dEntryNames::s_SurfaceDepthIoTblName = "SurfaceDepthIoTbl";
	const char* theProject3dEntryNames::s_SurfaceDepthIoAgeColName = "Age";

	const QString   theDefaults::InitialCrustThickness = "35000";
	const QString   theDefaults::InitialMantleThickness = "90000";
	const QString   theDefaults::SmoothingRadius = "0";
	const QString   theDefaults::RDA = "0.0";
	const QString   theDefaults::BasaltThickness = "7000.0";

	std::map<int, QString> CTCcontroller::PtModeMap = {
		{1, "Decompaction"},
		{2, "Hydrostatic"},
		{3, "It-Coupled"}
	};

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

		connect(ctcTab_->pushButtonCTCrunCTC(), SIGNAL(clicked()), this, SLOT(slotPushButtonCTCrunCtcClicked()));
		connect(ctcTab_->pushSelectProject3D(), SIGNAL(clicked()), this, SLOT(slotPushSelectProject3dClicked()));
		connect(ctcTab_->lineEditProject3D(), SIGNAL(textChanged(const QString&)), this, SLOT(slotLineEditProject3dTextChanged(const QString&)));
		connect(ctcTab_->pushButtonRunFastCauldron(), SIGNAL(clicked()), this, SLOT(slotpushButtonRunFastCauldronClicked()));
		connect(ctcTab_->lineEditNumProc(), SIGNAL(textChanged(const QString&)), this, SLOT(slotlineEditNumProcTextChanged(const QString&)));
		connect(ctcTab_->fastcldrnRunModeComboBox(), SIGNAL(currentIndexChanged(QString)), this, SLOT(slotRunModeComboBoxChanged(const QString&)));
		connect(ctcTab_->pushButtonCTCoutputMaps(), SIGNAL(clicked()), this, SLOT(slotpushButtonCTCoutputMapsClicked()));
		connect(ctcTab_->pushButtonExportCTCoutputMaps(), SIGNAL(clicked()), this, SLOT(slotpushButtonExportCTCoutputMapsClicked()));
	}



	int ctcWizard::CTCcontroller::findLastMapSqNumber()
	{
		auto GridMapIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_GridMapIoTblName);
		int LastmapSqNo = -1;
		for (int i = 0; i < GridMapIoTblSize; ++i) 
		{
			auto mapSqNo = cldrnProject.tableValueAsInteger("GridMapIoTbl", i, theProject3dEntryNames::s_GridMapIoMapSeqNbrColName);
			LastmapSqNo = mapSqNo > LastmapSqNo ? mapSqNo : LastmapSqNo;
#ifdef FUTURE_USE
			qDebug() << i << ' ' << LastmapSqNo;
#endif
		}
		return LastmapSqNo < 0 ? 0 : LastmapSqNo;
	}

	inline void CTCcontroller::setP3FileNameForCTCScenarios(QString name)
	{
		m_CTCP3FileName = name;
	}

	inline QString CTCcontroller::getP3FileNameForCTCScenarios(void) const
	{
		return m_CTCP3FileName;
	}


	QString CTCcontroller::createProject3dwithCTCUIinputs(const QString& scenarioFolderPath)
	{
		int startingMapSqNo = findLastMapSqNumber();
		startingMapSqNo++;
		
		// Cant change the extension if we use cmbApi
		setP3FileNameForCTCScenarios(scenarioFolderPath + "/" + mainController_->m_ctcP3FileName);

		mainController_->log("- New Project3d file created with CTC inputs under the newly created scenario folder");

		QFile newFile(m_CTCP3FileName);
		QFileInfo info(m_CTCP3FileName);
		if (info.exists()) newFile.remove();

		std::vector<QString> Basalt_map_names;
		std::vector<QString> RDA_map_names;
		std::vector<QString> RDAMapsUsed;
		std::vector<QString> BasaltMapsUsed;
		CreateRifthingHistoryBasaltMapUsedList(BasaltMapsUsed);
		CreateRifthingHistoryRDAMapList(RDAMapsUsed);
		//
		//insert Basalt and RDA maps used as the GridMapIoTbl inputs
		auto GridMapIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_GridMapIoTblName);
		for (const auto& aBasaltMap : BasaltMapsUsed)
		{
			if (aBasaltMap.compare(" ")) {
				Basalt_map_names.push_back("RiftMAPBasalt-" + QString::number(startingMapSqNo));
				long int mapSeqNbr = 0; // a new map has zero SqNo.
				cldrnProject.addRowToTable(theProject3dEntryNames::s_GridMapIoTblName);
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapReferredByColName, "CTCRiftingHistoryIoTbl");
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapNameColName, Basalt_map_names.back().toStdString());
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapTypeColName, "HDF5");
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapFileNameColName, aBasaltMap.toStdString());
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapSeqNbrColName, mapSeqNbr);
				GridMapIoTblSize++;
				++startingMapSqNo;
			}
			else
				Basalt_map_names.push_back("");
		}
		for (const auto& aRDAMap : RDAMapsUsed)
		{
			if (aRDAMap.compare(" ")) {
				RDA_map_names.push_back("RiftMAPRDA-" + QString::number(startingMapSqNo));
				long int mapSeqNbr = 0; // a new map has zero SqNo.
				cldrnProject.addRowToTable(theProject3dEntryNames::s_GridMapIoTblName);
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapReferredByColName, "CTCRiftingHistoryIoTbl");
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapNameColName, RDA_map_names.back().toStdString());
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapTypeColName, "HDF5");
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapFileNameColName, aRDAMap.toStdString());
				cldrnProject.setTableValue(theProject3dEntryNames::s_GridMapIoTblName, GridMapIoTblSize, theProject3dEntryNames::s_GridMapIoMapSeqNbrColName, mapSeqNbr);
				GridMapIoTblSize++;
				++startingMapSqNo;
			}
			else
				RDA_map_names.push_back("");
		}
		//
		//extracts basement age from StratIoTbl
		auto StratIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_StratIoTblName);
		auto BasementAge = cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_StratIoTblName, (size_t)StratIoTblSize - 1U, theProject3dEntryNames::s_StratIoDepoAgeColName);
		//
		//insert ContCrustalThicknessIoTbl inputs
		auto ContCrustalThicknessIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_ContCrustalThicknessIoTblName);
		if (ContCrustalThicknessIoTblSize == 0) {
			cldrnProject.addRowToTable(theProject3dEntryNames::s_ContCrustalThicknessIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoAgeColName, 0.0);
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoThicknessColName, ctcScenario_.lithosphereParameters()[0].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoThicknessGridColName, "");
		}
		else if (!NumericFunctions::isEqual(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, (size_t)ContCrustalThicknessIoTblSize - 1U, theProject3dEntryNames::s_ContCrustalThicknessIoAgeColName), BasementAge, Utilities::Numerical::DefaultNumericalTolerance)) {
			cldrnProject.addRowToTable(theProject3dEntryNames::s_ContCrustalThicknessIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoAgeColName, BasementAge);
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoThicknessColName, ctcScenario_.lithosphereParameters()[0].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, ContCrustalThicknessIoTblSize, theProject3dEntryNames::s_ContCrustalThicknessIoThicknessGridColName, "");
		}
		else {
			cldrnProject.setTableValue(theProject3dEntryNames::s_ContCrustalThicknessIoTblName, (size_t)ContCrustalThicknessIoTblSize - 1U, theProject3dEntryNames::s_ContCrustalThicknessIoThicknessColName, ctcScenario_.lithosphereParameters()[0].value.toDouble());
		}
		//
		//insert OceaCrustalThicknessIoTbl inputs
		auto OceaCrustalThicknessIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName);
		if (OceaCrustalThicknessIoTblSize == 0) {
			cldrnProject.addRowToTable(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoAgeColName, 0.0);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessColName, 0.0);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessGridColName, "");
		}
		else if (!NumericFunctions::isEqual(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, (size_t)OceaCrustalThicknessIoTblSize - 1U, theProject3dEntryNames::s_OceaCrustalThicknessIoAgeColName), BasementAge, Utilities::Numerical::DefaultNumericalTolerance)) {
			cldrnProject.addRowToTable(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoAgeColName, BasementAge);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessColName, 0.0);
			cldrnProject.setTableValue(theProject3dEntryNames::s_OceaCrustalThicknessIoTblName, OceaCrustalThicknessIoTblSize, theProject3dEntryNames::s_OceaCrustalThicknessIoThicknessGridColName, "");
		}
		//
		//insert CTCIoTbl inputs
		auto CTCIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_CTCIoTblName);
		if (!CTCIoTblSize) {
			cldrnProject.addRowToTable(theProject3dEntryNames::s_CTCIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoSurfaceNameColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoTRIniColName, DataAccess::Interface::DefaultUndefinedScalarValue); //this fields were moved to CTCReftingHistoryIoTbl, hence set to undefinedScalarValue
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoTRIniGridColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHCuIniColName, ctcScenario_.lithosphereParameters()[0].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHCuIniGridColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHLMuIniColName, ctcScenario_.lithosphereParameters()[1].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHLMuIniGridColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoDeltaSLColName, DataAccess::Interface::DefaultUndefinedScalarValue);//this fields were moved to CTCReftingHistoryIoTbl, hence set to undefinedScalarValue
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoDeltaSLGridColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHBuColName, DataAccess::Interface::DefaultUndefinedScalarValue);//this fields were moved to CTCReftingHistoryIoTbl, hence set to undefinedScalarValue
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoHBuGridColName, "");
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoFilterHalfWidthColName, ctcScenario_.lithosphereParameters()[2].value.toLong());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoUpperLowerContinentalCrustRatioColName, 0.5);
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, CTCIoTblSize, theProject3dEntryNames::s_CTCIoUpperLowerOceanicCrustRatioColName, 0.5);
		}		
		else
		{
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, (size_t)CTCIoTblSize - 1U, theProject3dEntryNames::s_CTCIoHCuIniColName, ctcScenario_.lithosphereParameters()[0].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, (size_t)CTCIoTblSize - 1U, theProject3dEntryNames::s_CTCIoHLMuIniColName, ctcScenario_.lithosphereParameters()[1].value.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCIoTblName, (size_t)CTCIoTblSize - 1U, theProject3dEntryNames::s_CTCIoFilterHalfWidthColName, ctcScenario_.lithosphereParameters()[2].value.toLong());

		}
		//
		//insert CTCRiftingHistoryIoTbl inputs
		int iRow = 0;
		int bastCnt = 0;
		int rdaCnt = 0;
		auto CTCRiftingHistoryIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName);
		for (const RiftingHistory& riftingHistory : ctcScenario_.riftingHistory())
		{
			QString RDAMap = "";
			QString RDAScalar = ctcScenario_.riftingHistory()[iRow].RDA;

			foreach(QString RDA_Map, ctcScenario_.riftingHistoryRDAMaps_)
			{
				if (RDA_Map == ctcScenario_.riftingHistory()[iRow].RDA_Map)
				{
					RDAMap = RDA_map_names[rdaCnt];
					++rdaCnt;
					break;
				}
			}
			QString BasaltMap = "";
			QString BasaltScalar = ctcScenario_.riftingHistory()[iRow].Basalt_Thickness;

			foreach(QString Basalt_Thck_Map, ctcScenario_.riftingHistoryBasaltMaps_)
			{
				if (Basalt_Thck_Map == ctcScenario_.riftingHistory()[iRow].Basalt_Thickness_Map)
				{
					BasaltMap = Basalt_map_names[bastCnt];
					++bastCnt;
					break;
				}
			}
			if (!CTCRiftingHistoryIoTblSize)
				cldrnProject.addRowToTable(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName);
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoAgeColName, ctcScenario_.riftingHistory()[iRow].Age.toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoTectonicFlagColName, (ctcScenario_.riftingHistory()[iRow].TectonicFlag).toStdString());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLColName, (RDAScalar).toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLGridColName, (RDAMap).toStdString());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoHBuColName, (BasaltScalar).toDouble());
			cldrnProject.setTableValue(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, iRow, theProject3dEntryNames::s_CTCRiftingHistoryIoHBuGridColName, (BasaltMap).toStdString());
			iRow++;
		}
		cldrnProject.saveModelToProjectFile(m_CTCP3FileName.toStdString(), true);
		//
		return m_CTCP3FileName;
	}



	void ctcWizard::CTCcontroller::CreateRifthingHistoryBasaltMapUsedList(std::vector<QString>& BasaltMapsUsed) const
	{
		int iRow = 0;
		for (const RiftingHistory& riftingHistoryEntry : ctcScenario_.riftingHistory())
		{
			foreach(QString Basalt_Thck_Map, ctcScenario_.riftingHistoryBasaltMaps_)
			{
				if (Basalt_Thck_Map == riftingHistoryEntry.Basalt_Thickness_Map)
				{
					BasaltMapsUsed.push_back(Basalt_Thck_Map);
				}
			}
			++iRow;
		}
	}

	void ctcWizard::CTCcontroller::CreateRifthingHistoryRDAMapList(std::vector<QString>& RDAMapsUsed) const
	{
		int iRow = 0;
		for (const RiftingHistory& riftingHistory : ctcScenario_.riftingHistory())
		{
			foreach(QString RDA_Map, ctcScenario_.riftingHistoryRDAMaps_)
			{
				if (RDA_Map == riftingHistory.RDA_Map)
				{
					RDAMapsUsed.push_back(RDA_Map);
				}
			}
			++iRow;
		}
	}

	void CTCcontroller::updateProjectTxtFile(const QString& scenarioFolderPath) const
	{

		QString oldProjectTxtFile = scenarioFolderPath + "/Project.txt";
		QString newProjectTxtFile = scenarioFolderPath + "/Project.txt_1";

		QFile oldFile(oldProjectTxtFile);
		QFile newFile(newProjectTxtFile);

		QFileInfo info(newProjectTxtFile);

		if (info.exists())
		{
			newFile.remove();
		}

		bool hasOpened = oldFile.open(QIODevice::ReadOnly | QIODevice::Text);
		if (hasOpened)
		{
			newFile.open(QIODevice::ReadWrite | QIODevice::Text);

			QTextStream oldStream(&oldFile);
			QTextStream newStream(&newFile);
			QString line = oldStream.readLine();


			while (!line.isNull()) {
				//Scenario
				QStringList strList = line.simplified().split(":");
				QString prefix_ = "CTCv2-";
				if (!strList[0].compare("Scenario", Qt::CaseSensitive)) {

					newStream << "Scenario: " << strList[1] << '-' << qtutils::getTimeStamp(prefix_) << endl;
				}
				//Project
				else if (!strList[0].compare("Project", Qt::CaseSensitive)) {
					newStream << "Project: " << strList[1] << '-' << qtutils::getTimeStamp(prefix_) << endl;
				}
				//Top Boundary Conditions
				else if (strList[0].contains("Top Boundary Conditions", Qt::CaseSensitive)) {
					newStream << "Top Boundary Conditions: Updated by CTC Outputs " << endl;
				}

				//Bottom Boundary Conditions
				else if (strList[0].contains("Bottom Boundary Conditions", Qt::CaseSensitive)) {
					newStream << "Bottom Boundary Conditions: Updated by CTC Outputs " << endl;
				}
				//Description
				else if (strList[0].contains("Description", Qt::CaseSensitive)) {
					newStream << "Description: " << strList[1] << " " << qtutils::getTimeStamp(prefix_) << endl;
				}
				else
					newStream << line << endl;
				line = oldStream.readLine();
			}

			oldFile.close();
			newFile.close();

			oldFile.remove();
			newFile.rename(oldProjectTxtFile);
			mainController_->log("- New Project.txt file prepared for scenario import!");
		}
		else {
			mainController_->log("**************************************** BASIN WARNING ****************************************");
			mainController_->log("*********************************** Invalid/Missing Project.txt ***************************************");
			mainController_->log("*************************** This scenario can not be imported to BPA2 Basin! ******************************");
			mainController_->log("*********************************************************************************************");
		}
	}

	mbapi::Model::ReturnCode CTCcontroller::upateProject3dfileToStoreOutputs(const QString& project3dFilePath)
	{
		auto tbls = cldrnProject.tableSize("FilterTimeIoTbl");
		int count = 0;
		for (int ii = 0; ii < tbls; ii++)
		{
			if(!cldrnProject.tableValueAsString("FilterTimeIoTbl", ii, "PropertyName").compare("LithoStaticPressure"))
				count++;
		}

		// if there is not at least one entry, enforce it...
		if (count < 1)
		{
			auto errCode = cldrnProject.addRowToTable("FilterTimeIoTbl");
			if (errCode != mbapi::Model::ReturnCode::NoError) 
			{
				errCode = cldrnProject.setTableValue("FilterTimeIoTbl", (size_t)cldrnProject.tableSize("FilterTimeIoTbl") - 1U, "PropertyName", "LithoStaticPressure");
				cldrnProject.setTableValue("FilterTimeIoTbl", (size_t)cldrnProject.tableSize("FilterTimeIoTbl") - 1U, "OutputOption", "SedimentsPlusBasement");
				cldrnProject.setTableValue("FilterTimeIoTbl", (size_t)cldrnProject.tableSize("FilterTimeIoTbl") - 1U, "ResultOption", "Simple");
			}
		}
		// We can not use cmbiApi to write new scenario with saveModelToProjectFile method,
		// if we don't have the simulation outputs in the same P3 file in the OutputTbls.
		cldrnProject.clearTable("OutputTablesFileIoTbl");

		auto err = cldrnProject.clearTable("OutputTablesIoTbl");
		//From here the HDF file name is sourced by "saveModelToProjectFile"
		cldrnProject.clearTable("TimeIoTbl");

		// clear previous results
#ifdef FUTURE_USE
		cldrnProject.clearTable("3DTimeIoTbl");
		cldrnProject.clearTable("SnapshotIoTbl");
#endif
		err = cldrnProject.addRowToTable("OutputTablesIoTbl");
		cldrnProject.setTableValue("OutputTablesIoTbl", (size_t)cldrnProject.tableSize("OutputTablesIoTbl") - 1U, "TableName", "TimeIoTbl");

		err = cldrnProject.addRowToTable("OutputTablesIoTbl");
		cldrnProject.setTableValue("OutputTablesIoTbl", (size_t)cldrnProject.tableSize("OutputTablesIoTbl") - 1U, "TableName", "3DTimeIoTbl");

		err = cldrnProject.addRowToTable("OutputTablesIoTbl");
		cldrnProject.setTableValue("OutputTablesIoTbl", (size_t)cldrnProject.tableSize("OutputTablesIoTbl") - 1U, "TableName", "SnapshotIoTbl");

		err = cldrnProject.addRowToTable("OutputTablesIoTbl");
		cldrnProject.setTableValue("OutputTablesIoTbl", (size_t)cldrnProject.tableSize("OutputTablesIoTbl") - 1U, "TableName", "SimulationDetailsIoTbl");

		// write the new p3 file by using this method
		auto strList = project3dFilePath.split(".project3d");
		auto newName = strList[0] + "ctcwiz.project3d";
#ifdef FUTURE_USE
		return cldrnProject.saveModelToProjectFile(newName.toStdString());
#endif
		return cldrnProject.saveModelToProjectFile(project3dFilePath.toStdString().c_str());
	}



	bool CTCcontroller::validateCTCinputFields() const
	{
		bool isValidFieldValues = true;
		bool validate;
		double value;

		//[CTCIoTbl]
		value = ctcScenario_.lithosphereParameters()[0].value.toDouble(&validate);
		if (!validate)
		{
			mainController_->log("- Invalid Input for: Initial Crust Thickness");
			isValidFieldValues = false;
		}

		value = ctcScenario_.lithosphereParameters()[1].value.toDouble(&validate);
		if (!validate)
		{
			mainController_->log("- Invalid Input for: Initial Mantle Thickness");
			isValidFieldValues = false;
		}
		// Smoothing Radius is actually the no. of cells to consider for map smoothing
		value = ctcScenario_.lithosphereParameters()[2].value.toInt(&validate);
		if (!validate)
		{
			mainController_->log("- Invalid Input for: Smoothing Radius");
			isValidFieldValues = false;
		}

		//[CTCRiftingHistoryIoTbl]

		for (const RiftingHistory& riftingHistory : ctcScenario_.riftingHistory())
		{
			value = riftingHistory.RDA.toDouble(&validate);
			if (!validate)
			{
				mainController_->log("- Invalid Input for RDA Adjustment in Rifting History Table; for Age: " + riftingHistory.Age);
				isValidFieldValues = false;
			}
			value = riftingHistory.Basalt_Thickness.toDouble(&validate);
			if (!validate)
			{
				mainController_->log("- Invalid Input for Max Basalt Thickness in Rifting History Table; for Age: " + riftingHistory.Age);
				isValidFieldValues = false;
			}

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
		if (ctcScenario_.project3dPath().isEmpty())
			mainController_->log("- Please select project3d file");
		else
		{
			// the cmbAPI cldrnProject.saveModelToProjectFile() does not copy the outputTable entries and they turn up as empty fields
			// hence the results are not copied to ctc 1,2,3... dirs, in that case the HDFs are cont copied there
			// Load the newly written p3file that has some change (if at all)
			cldrnProject.loadModelFromProjectFile(ctcScenario_.project3dPath().toStdString());
			bool isValidFieldValues = validateCTCinputFields();
			if (ctcScenario_.runMode().isEmpty())
				fastcldrnRunMode = "Decompaction";
			else
				fastcldrnRunMode = ctcScenario_.runMode();

			auto check = qtutils::isValidNoOfProcs(ctcScenario_.numProc());

			if (check.compare("Ok"))
			{
				mainController_->log("- Invalid Input for: Number of Processors (CTC)...");
				mainController_->log(check);
				isValidFieldValues = false;
			}

			if (isValidFieldValues)
			{
				auto isResultsAvailable = isFastCauldronResultsAvailable(&fastcldrnRunMode);

				if (!isResultsAvailable)
				{
					mainController_->log("- Cauldron results do not exist, or complete results up to present day not available for Cauldron mode: " + fastcldrnRunMode);
					mainController_->log("- !!!Run Cauldron mode: " + fastcldrnRunMode + ", or select another Cauldron mode and then try CTC again!");
				}
				else
				{
					auto ctcFilePath = ctcScenario_.ctcFilePathCTC();
					QString scenarioFolderPath = mainController_->createCTCscenarioFolder(ctcScenario_.project3dPath());

					QString ctcFilenameWithPath = createProject3dwithCTCUIinputs(scenarioFolderPath);
					auto ctcOutPath = scenarioFolderPath + "/" + mainController_->m_ctcDirName;
					QDir ctcDir(ctcOutPath);
					auto list = ctcDir.entryList(QStringList() << "*.hdf" << "*.HDF", QDir::Files);

					if (!list.isEmpty())
					{
						mainController_->executeCtcScript(ctcFilenameWithPath, ctcScenario_.numProc());
					}
					else
					{
						mainController_->log("Something went wrong with Pressure run, the HDF file was not copied to " + ctcOutPath); return;
					}
					mainController_->mapOutputCtcScript(ctcFilenameWithPath);
					updateProjectTxtFile(scenarioFolderPath);
				}
			}
		}

	}

	int CTCcontroller::isFastCauldronResultsAvailable(const QString* fastcldrnRunMode) const
	{
		QString project3dFile = ctcTab_->lineEditProject3D()->text();
		QFileInfo info(project3dFile);
		QStringList strLst = info.fileName().simplified().split(".");
		QDir dir(info.dir().absolutePath() + "/" + strLst[0] + "_CauldronOutputDir");

		if (!dir.exists()) 
		{
			return 0;// nothing is run
		}

		//collect fastcauldron output files from project3d file and check whether these files exist in output folder
#ifdef FUTURE_USE
		SnapshotIoTbl Table
			QVector<QString> SnapshotIoTblVector;
		QString tblName = "[SnapshotIoTbl]";
		GetTableFromProject3d(project3dFile, SnapshotIoTblVector, tblName);

		for (int i = 0; i < SnapshotIoTblVector.size(); ++i)
		{
			QStringList strLst1 = SnapshotIoTblVector[i].simplified().split(" ");
			QStringList fileLst = strLst1[strLst1.size() - 1].simplified().split("\"");
			QFile resFile(dir.filePath(fileLst[1]));
			if (!resFile.exists()) return 0; // nothing in SnapshotIoTbl
		}
#endif
		// if you don't pass a valid pointer, then it checks any				   
		int type = 0;
		if (fastcldrnRunMode)
		{
			if ((*fastcldrnRunMode).contains("Decompaction"))
			{
				QFile resFile(dir.filePath("HydrostaticDecompaction_Results.HDF"));
				if (resFile.exists() && QFile::exists(dir.filePath("Time_0.000000.h5")))
					type = 1;
			}
			else if ((*fastcldrnRunMode).contains("Hydrostatic"))
			{
				QFile resFile(dir.filePath("HydrostaticTemperature_Results.HDF"));
				if (resFile.exists() && QFile::exists(dir.filePath("Time_0.000000.h5")))
					type = 2;
			}
			else if ((*fastcldrnRunMode).contains("Iteratively Coupled"))
			{
				QFile resFile(dir.filePath("PressureAndTemperature_Results.HDF"));
				if (resFile.exists() && QFile::exists(dir.filePath("Time_0.000000.h5")))
					type = 3;
			}
		}
		else // for any T/P results
		{
			if (QFile::exists(dir.filePath("HydrostaticDecompaction_Results.HDF")) && QFile::exists(dir.filePath("Time_0.000000.h5")))
			{
				type = 1;
			}
			if (QFile::exists(dir.filePath("HydrostaticTemperature_Results.HDF")) && QFile::exists(dir.filePath("Time_0.000000.h5")))
			{
				type = 2;
			}
			if (QFile::exists(dir.filePath("PressureAndTemperature_Results.HDF")) && QFile::exists(dir.filePath("Time_0.000000.h5")))
			{
				type = 3;
			}
		}

		return type;// Something was found, is it all completed, I don't know!
	}

	void CTCcontroller::slotpushButtonRunFastCauldronClicked()
	{
		//
		if (ctcScenario_.project3dPath().isEmpty())
			mainController_->log("- Please select project3d file");
		else
		{
			bool isValidFieldValues = true;
			QString fastcldrnRunMode;

			if (ctcScenario_.runMode().isEmpty())
				fastcldrnRunMode = "Decompaction";
			else
				fastcldrnRunMode = ctcScenario_.runMode();

			auto check = qtutils::isValidNoOfProcs(ctcScenario_.numProc());

			if (check.compare("Ok"))
			{
				mainController_->log("- Invalid Input for: Number of Processors (Cauldron)...");
				mainController_->log(check);
				isValidFieldValues = false;
			}

			if (isValidFieldValues)
			{
				upateProject3dfileToStoreOutputs(ctcScenario_.project3dPath());
				auto isResultsAvailable = isFastCauldronResultsAvailable(&fastcldrnRunMode);
				if (!isResultsAvailable)
				{
					mainController_->log("- Cauldron results do not exist for Cauldron mode: " + fastcldrnRunMode);
					mainController_->executeFastcauldronScript(ctcScenario_.project3dPath(), fastcldrnRunMode, ctcScenario_.numProc());
				}
				else
				{
					mainController_->log("- Cauldron results already exists for Cauldron mode: " + fastcldrnRunMode);
				}
			}
		}

	}

	void CTCcontroller::slotpushButtonExportCTCoutputMapsClicked()
	{
		QString scenarioFolder = QFileDialog::getExistingDirectory(ctcTab_, "Select CTC Scenario Folder which contains CTC output maps",
			ctcScenario_.project3dPath());
		mainController_->log("");
		mainController_->log("Selected CTC Scenario: " + scenarioFolder);

		QDir scenarioDirec(scenarioFolder);
		QStringList fileList = scenarioDirec.entryList(QStringList() << "*_OUT.project3d", QDir::Files);
		if (fileList.empty())
		{
			mainController_->log("");
			mainController_->log("- Selected Scenario folder doesn't contain CTC output project file!!!");
			mainController_->log("- Please select valid CTC Scenario folder!!!");
		}
		else
		{
			QStringList strLst = fileList[0].split(".");
			QStringList ctcOutputDir = scenarioDirec.entryList(QStringList() << strLst[0] + "_CauldronOutputDir", QDir::AllEntries);
			if (ctcOutputDir.empty())
			{
				//
				mainController_->log("");
				mainController_->log("- Selected Scenario folder doesn't contain CTC output directory!!!");
				mainController_->log("- Please select valid CTC Scenario folder!!!");
			}
			else
			{
				mainController_->log("");
				mainController_->log("This CTC Scenario will be used to Create scenario for BPA2-Basin: ");
				bool success = mainController_->createScenarioForALC(scenarioFolder);

				if (success && QString(qgetenv("ADVANCED_USER")).compare("Yes", Qt::CaseInsensitive))
					mainController_->deleteCTCscenario(scenarioFolder);

			}
		}
	}

	void CTCcontroller::slotPushSelectProject3dClicked()
	{
		// this was a botch up; max fileName on Lnux is 255 chars & maximum path of 4096 characters
		int iMaxPathLen = 4096;             // Maximum path length
		int iMaxProject3dFileLen = 239;     // Maximum file name a Project3d can safely assume

		// Getting the project3d file name with path
		QString fileName = QFileDialog::getOpenFileName(ctcTab_, "Select project file", ctcScenario_.project3dPath(),
			"Project files (*.project3d)");
		if (fileName.isEmpty())
		{
			mainController_->log("No project3d file selected, try again!!");
			return;
		}
		QFileInfo p3dinfo(fileName);
		// Checking if the input project3d file name length is within valid range
		if (p3dinfo.fileName().length() > iMaxProject3dFileLen) {
			mainController_->log("Selected Project3d file: " + p3dinfo.fileName());
			mainController_->log("Selected Project3d filename can not exceeds 239 characters!!! Shorten the Project3d file name!!!");
			return;
		}

		// Checking if the scenario path length is within valid range
		if (fileName.length() > iMaxPathLen)
		{
			mainController_->log("Selected Project3d file with path: " + fileName);
			mainController_->log("Selected Scenario Path is too lengthy to handle!!! Shorten the folder names and path!!!");
			return;
		}

		ctcTab_->lineEditProject3D()->setText(fileName);

		ctcScenario_.clearLithosphereTable();
		ctcScenario_.clearRiftingHistoryTable();

		LithosphereParameterController LpC(ctcTab_->lithosphereParameterTable(), ctcScenario_, mainController_);
		RiftingHistoryController RHC(ctcTab_->riftingHistoryTable(), ctcScenario_, mainController_);

		QString strHcuIni;
		QString strHLMuIni;
		QString strFilterHalfWidth;
				
		auto CTCIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_CTCIoTblName);
		if (CTCIoTblSize)
		{
			// if an entry already exists then delete it
			for (int i = 0; i < CTCIoTblSize -1; ++i)
			{
				cldrnProject.removeRecordFromTable(theProject3dEntryNames::s_CTCIoTblName, i);
			}
			strHcuIni = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_CTCIoTblName, 0, theProject3dEntryNames::s_CTCIoHCuIniColName));
			strHLMuIni = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_CTCIoTblName, 0, theProject3dEntryNames::s_CTCIoHLMuIniColName));
			strFilterHalfWidth = QString::number(cldrnProject.tableValueAsInteger(theProject3dEntryNames::s_CTCIoTblName, 0, theProject3dEntryNames::s_CTCIoFilterHalfWidthColName));
			ctcScenario_.addLithosphereParameter("Initial Crust Thickness [m]", strHcuIni);
			ctcScenario_.addLithosphereParameter("Initial Mantle Thickness [m]", strHLMuIni);
			ctcScenario_.addLithosphereParameter("Smoothing Radius [number of cells]", strFilterHalfWidth);
		}
		else
		{
			ctcScenario_.addLithosphereParameter("Initial Crust Thickness [m]", theDefaults::InitialCrustThickness);
			ctcScenario_.addLithosphereParameter("Initial Mantle Thickness [m]", theDefaults::InitialMantleThickness);
			ctcScenario_.addLithosphereParameter("Smoothing Radius [number of cells]", theDefaults::SmoothingRadius);
		}

		QString strHasPWD;
		QVector<QString> strVectHasPWD;
		auto SurfaceDepthIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_SurfaceDepthIoTblName);
		if (SurfaceDepthIoTblSize) {
			for (int i = 0; i < SurfaceDepthIoTblSize; ++i)
			{
				strVectHasPWD << QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_SurfaceDepthIoTblName, i, theProject3dEntryNames::s_SurfaceDepthIoAgeColName));
			}
		}

		auto CTCRiftingHistoryIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName);
		if (CTCRiftingHistoryIoTblSize)
		{
			for (int i = 0; i < CTCRiftingHistoryIoTblSize; ++i)
			{
				strHasPWD = "N";
				auto strAge = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoAgeColName));
				auto strTectonicFlg = QString::fromStdString(cldrnProject.tableValueAsString(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoTectonicFlagColName));
				auto strRDA = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLColName));
				auto strRDAMap = QString::fromStdString(cldrnProject.tableValueAsString(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoDeltaSLGridColName));
				auto strBasalt = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoHBuColName));
				auto strBasaltMap = QString::fromStdString(cldrnProject.tableValueAsString(theProject3dEntryNames::s_CTCRiftingHistoryIoTblName, i, theProject3dEntryNames::s_CTCRiftingHistoryIoHBuGridColName));
				for (int j = 0; j < strVectHasPWD.size(); ++j)
				{
					if (!strVectHasPWD[j].compare(strAge, Qt::CaseInsensitive))
					{
						strHasPWD = "Y";
						break;
					}
				}
				ctcScenario_.addRiftingHistory(strAge, strHasPWD, strTectonicFlg, strRDA, strRDAMap, strBasalt, strBasaltMap);
			}
		}
		else
		{
			auto StratIoTblSize = cldrnProject.tableSize(theProject3dEntryNames::s_StratIoTblName);
			for (int i = 0; i < StratIoTblSize; ++i)
			{
				strHasPWD = "N";
				auto strDepoAge = QString::number(cldrnProject.tableValueAsDouble(theProject3dEntryNames::s_StratIoTblName, i, theProject3dEntryNames::s_StratIoDepoAgeColName));

				for (int j = 0; j < strVectHasPWD.size(); ++j)
				{
					if (!strVectHasPWD[j].compare(strDepoAge, Qt::CaseInsensitive))
					{
						strHasPWD = "Y";
						break;
					}
				}

				if (i == 0)
					ctcScenario_.addRiftingHistory(strDepoAge, strHasPWD, "Flexural Basin", theDefaults::RDA, "", theDefaults::BasaltThickness, "");
				else if (i == StratIoTblSize - 1)
					ctcScenario_.addRiftingHistory(strDepoAge, strHasPWD, "Active Rifting", theDefaults::RDA, "", theDefaults::BasaltThickness, "");
				else
					ctcScenario_.addRiftingHistory(strDepoAge, strHasPWD, "Passive Margin", theDefaults::RDA, "", theDefaults::BasaltThickness, "");
			}
		}

		QFileInfo info(ctcScenario_.project3dPath());
		QDir dir(info.absoluteDir().path());
		QStringList filters;
		filters << "*.hdf" << "*.hdf5" << "*.h5"; // case doesn't matter

		ctcScenario_.riftingHistoryBasaltMaps_ = QStringList() << " ";
		ctcScenario_.riftingHistoryRDAMaps_ = QStringList() << " ";

		foreach(QString file, dir.entryList(filters, QDir::Files))
		{
			// This arbitrarily assigns all Hdf files to these variable,
			// a hack to have the list in the drop-down menu of the fields
			if (file.compare("Inputs.HDF"))// Exclude Inputs.HDF from this list
			{
				ctcScenario_.riftingHistoryRDAMaps_ << file;
				ctcScenario_.riftingHistoryBasaltMaps_ << file;
			}

		}

		LithosphereParameterController LPC(ctcTab_->lithosphereParameterTable(), ctcScenario_, mainController_);
		RiftingHistoryController rHC(ctcTab_->riftingHistoryTable(), ctcScenario_, mainController_);

		mainController_->log("Selected Scenario: \"" + info.absoluteDir().path() + "\"");
		mainController_->log("Loaded project3d file in CTC-UI: \"" + info.fileName() + "\"");
		mainController_->log("- If RDA Adjustment Maps or Basalt Thickness Maps are selected, then respective scalar values are not considered !!!");

		// save any changes here
		cldrnProject.saveModelToProjectFile(ctcScenario_.project3dPath().toStdString());
	}


	void CTCcontroller::slotLineEditProject3dTextChanged(const QString& project3dPath)
	{
		// whats the p3 file name that was selected?
		QFileInfo info1(project3dPath);
		QStringList strList = info1.fileName().simplified().split(".");
		// As soon as the originalFileName is determined we are set to
		// obtain all other required file_names and output folder names
		mainController_->m_originalP3FileName = strList[0] + ".project3d";
		mainController_->m_ctcP3FileName = strList[0] + "CTC" + ".project3d";
		mainController_->m_ctcOutputFileName = strList[0] + "CTC_OUT" + ".project3d";
		mainController_->m_ctcDirName = strList[0] + "CTC" + "_CauldronOutputDir";
		mainController_->m_ctcOutputDirName = strList[0] + "CTC" + "_OUT_CauldronOutputDir";

		ctcScenario_.setProject3dPath(project3dPath);
		if (ErrorHandler::NoError != cldrnProject.loadModelFromProjectFile(project3dPath.toStdString()))
		{
			mainController_->log("Failing to load project file: " + project3dPath);
		}
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
