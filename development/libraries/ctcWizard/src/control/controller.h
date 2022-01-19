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
		void launchCauldronMapsTool(const QString& filePath);

		void log(const QString& text)const;


		void createScenarioForALC(const QString& folderPath);
		void FinalizeProject3dFile(const QString& oldFile, const QString& newFile)const;
		void deleteCTCscenario(const QString& folderPath);
		void showUI();

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
#ifdef DEBUG_CTC_OTHERS
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
