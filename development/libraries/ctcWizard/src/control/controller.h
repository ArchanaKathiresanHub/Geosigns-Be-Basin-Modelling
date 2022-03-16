#pragma once
#include <QObject>
#include <QDir>
#include <QProcess>


#include "model/ctcScenario.h"
#include "view/mainWindow.h"
#include "SDUWorkLoadManager.h"
#include "Qt_Utils.h"


#define WAITTIME 500

class QProcess;
class QString;
class QTableWidgetItem;

namespace ctcWizard
{
	const QString outLog("out.log");
	const QString errLog("err.log");


	class Controller : public QObject
	{
		Q_OBJECT

	public:
		explicit Controller();
		void executeFastcauldronScript(const QString& filePath, const QString& fastcldrnRunMode, const QString numProc = "1") const;
		void executeCtcScript(const QString& filePath, const QString numProc = "1") const;
		QString createCTCscenarioFolder(const QString& filePath) const;
		void mapOutputCtcScript(const QString& filePath) const;
		void launchCauldronMapsTool(const QString& filePath) const;

		void log(const QString& text)const;
		QString m_originalP3FileName;
		QString m_ctcP3FileName;
		QString m_ctcDirName;
		QString m_ctcOutputFileName;
		QString m_ctcOutputDirName;

		bool createScenarioForALC(const QString& folderPath) const;
		/// clean the RDA and Basalt Maps from p3 file since BPA2-Basin does not have map-type for the same import will fail
		void finalizeProject3dFile(const QString& oldFile, const QString& newFile)const;
		void deleteCTCscenario(const QString& folderPath) const;
		void showUI();
		static const int MaxDirCount;

	private slots:
		void slotActionExitTriggered();
		void slotActionOpenFileTriggered();
		void slotPushClearLogClicked(void) const;
		void  processError(QProcess::ProcessError error) const;
		void  processFinished(int exitCode, QProcess::ExitStatus exitStatus, const qtutils::FileLogger& fl,
			const QString& logFilePath, QTextEdit* theText, workloadmanagers::WorkLoadManagerType wlmType = workloadmanagers::WorkLoadManagerType::AUTO) const;

	private:
		void createConnections() const;
		bool processCommand(QProcess& process, const QString& command) const;
#ifdef FUTURE_USE
		bool processShCommand(QProcess& process, const QString& command) const;
#endif
		bool makeDirSymLinks(const QDir& src, const QDir& desti) const;
		MainWindow ui_;
		CtcScenario ctcScenario_;

		// This is a new CTCcontroller
		QVector<QObject*> subControllers_;

	signals:
	};

} // namespace ctcWizard
