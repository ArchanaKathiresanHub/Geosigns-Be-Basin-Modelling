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
#include <QScrollBar>


#include "Qt_Utils.h"					 
namespace ctcWizard
{
	const int Controller::MaxDirCount = 100;


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
		connect(ui_.actionExit(), SIGNAL(triggered()), this, SLOT(slotActionExitTriggered()));
		connect(ui_.pushClearLog(), SIGNAL(clicked()), this, SLOT(slotPushClearLogClicked()));
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

		QString fileToLog = outLog;
		QProcess process;
		connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });

		QEventLoop loop;
		connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), &loop, &QEventLoop::quit);
		QFileInfo info(filePath);
		process.setWorkingDirectory(info.absoluteDir().path());

		QTimer timer2;
		qtutils::FileLogger myLogger;
		connect(&timer2, &QTimer::timeout, this, [&] { myLogger.logFromFile(info.absoluteDir().path() + "/" + fileToLog, ui_.lineEditLog()); });
		timer2.start(WAITTIME);

		QDir dir(info.dir().absolutePath() + "/" + m_originalP3FileName.split(".project3d")[0] + "_CauldronOutputDir");
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
		QString CLDRN_BIN = getenv("CLDRN_BIN");
		if (CLDRN_BIN.isEmpty()) {
			qDebug() << "no CLDRN_BIN defined!";
		}
		else
		{
			CLDRN_BIN += QString("\\fastcauldron.exe");
		}
		// This path is added during MSMPI installation
		QString MPI_BIN = getenv("MSMPI_BIN") + QString("mpiexec.exe");
		QString MPI_OPTIONS = "-logfile cauldron-mpi-output-rank-0.log";
		fileToLog = "cauldron-mpi-output-rank-0.log";
#else
		//The following will now call whatever is loaded in the OS PATH variable
		QString CLDRN_BIN = qtutils::exportApplicationPath() + '/' + "fastcauldron";
		QString MPI_BIN = "mpirun";
		QString MPI_OPTIONS = "";//"-outfile-pattern 'cauldron-mpi-output-rank-%r.log'";
#endif // Q_OS_WIN

		auto cwd = process.workingDirectory();
		auto wlm = workloadmanagers::WorkLoadManager::Create(cwd.toStdString() + "/cldrn.sh", workloadmanagers::WorkLoadManagerType::AUTO);
		connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
			this, [&] {Controller::processFinished(process.exitCode(), process.exitStatus(), myLogger, info.absoluteDir().path() + "/" + fileToLog
				, ui_.lineEditLog(), wlm->getWlmType()); });

		QString runPT = QString::fromStdString
		(
			wlm->JobSubmissionCommand
			(
				"cldrn", "", 1800, "ctcPressureJob", fileToLog.toStdString(),
				"", numProc.toStdString(), "", "", qtutils::addDoubleQuotes(process.workingDirectory()).toStdString(), false, false,
				(qtutils::addDoubleQuotes(MPI_BIN).toStdString() + " -n " +
					numProc.toStdString() + ' ' + (MPI_OPTIONS).toStdString() + ' ' + qtutils::addDoubleQuotes(CLDRN_BIN).toStdString() +
					" -project " + qtutils::addDoubleQuotes(filePath).toStdString() + " " + cldrnRunMode.toStdString()
					),
				true
			)
		);

		log("- The CWD is: " + process.workingDirectory());

#ifdef FUTURE_USE
		// processCommand does not work for //bsub <myJobs.sh, because i/o redirection is handled by a shell, not by QProcess or the external exe
		auto isOk = processShCommand(process, runPT);
#else
		auto isOk = processCommand(process, runPT);
#endif

		if (isOk)
			log("- Wait for Cauldron Computations...");
		loop.exec(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
	}

	//=============================================================
	void Controller::executeCtcScript(const QString& ctcFilenameWithPath, const QString numProc) const
	{
		log("- Start running ctc");
		QTimer timer;
		timer.start(100);
		while (timer.remainingTime() > 0)
		{
			qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
		}

		QString fileToLog = outLog;

		QFileInfo info(ctcFilenameWithPath);
		QDir baseDir(info.absoluteDir().path());

		if (info.exists())
		{
			baseDir.cdUp();
		}

		QDir scenarioDirec(info.absoluteDir().path());

		QStringList fileList = baseDir.entryList(QStringList() << "Project.txt" << "*.hdf" << "*.HDF", QDir::Files);
		foreach(QString file, fileList)
		{
			if (file.compare("Inputs.HDF"))
				QFile::copy(baseDir.path() + "/" + file, scenarioDirec.path() + "/" + file);
		}
		QStringList strList = info.fileName().simplified().split(".");

		//Create Project_CauldronOutputDir in the CTC scenario working dir to copy the fastcauldron output files
		QString fastCtcOutputDirPath = scenarioDirec.path() + "/" + strList[0] + "_CauldronOutputDir";
		QDir fastCtcOutputDir(fastCtcOutputDirPath);
		if (!fastCtcOutputDir.exists())//not absolutely needed 
		{
			//create it
			fastCtcOutputDir.mkpath(fastCtcOutputDirPath);
		}

		QString fastcauldronOutputDirPath = baseDir.path() + "/" + strList[0] + "_CauldronOutputDir";
		QDir fastcauldronOutputDir(fastcauldronOutputDirPath);
		// Some files, whether they are all that we meed? I dunno.
		if (fastcauldronOutputDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries).count() == 0)
			log("- fastcauldron output dir is empty");
#ifdef Q_OS_WIN
		/// Define the Usual environment Variables in the VS project and also add the following the the PATH of fastctc.exe
		QString CTC_BIN = getenv("CTC_BIN");
		if (CTC_BIN.isEmpty()) {
			qDebug() << "no CTC_BIN defined!";
		}
		else
		{
			CTC_BIN += QString("\\fastctc.exe");
		}
		// This path is added during MSMPI installation
		QString MPI_BIN = getenv("MSMPI_BIN") + QString("mpiexec.exe");
		QString MPI_OPTIONS = "-logfile ctc-mpi-output-rank-0.log";
		fileToLog = "ctc-mpi-output-rank-0.log";
#else
		//The following will now call whatever is loaded in the OS PATH variable
		QString CTC_BIN = qtutils::exportApplicationPath() + '/' + "fastctc";
		QString MPI_BIN = "mpirun";
		QString MPI_OPTIONS = "";//"-outfile-pattern 'ctc-mpi-output-rank-%r.log'";
#endif // WIN32
		QProcess process;
		/// ctc runs too fast for log to get updated in the UI.
		QEventLoop loop;
		connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), &loop, &QEventLoop::quit);

		process.setWorkingDirectory(scenarioDirec.path());

		QTimer timer2;
		qtutils::FileLogger myLogger;
		connect(&timer2, &QTimer::timeout, this, [&] { myLogger.logFromFile(info.absoluteDir().path() + "/" + fileToLog, ui_.lineEditLog()); });
		timer2.start(WAITTIME);
		connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });

		auto cwd = process.workingDirectory();
		auto wlm = workloadmanagers::WorkLoadManager::Create(cwd.toStdString() + "/cldrn.sh", workloadmanagers::WorkLoadManagerType::AUTO);
		auto saveP3 = qtutils::addDoubleQuotes(info.absoluteDir().path() + "/" + strList[0] + "_OUT.project3d").toStdString();

		connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
			this, [&] {Controller::processFinished(process.exitCode(), process.exitStatus(), myLogger, info.absoluteDir().path() + "/" + fileToLog
				, ui_.lineEditLog(), wlm->getWlmType()); });

		QString runCTC = QString::fromStdString
		(
			wlm->JobSubmissionCommand
			("cldrn", "", 1800, "ctcCalcJob", outLog.toStdString(), "", numProc.toStdString(), "", "",
				qtutils::addDoubleQuotes(process.workingDirectory()).toStdString(), false, false,
				(
					qtutils::addDoubleQuotes(MPI_BIN).toStdString() + " -n " +
					numProc.toStdString() + ' ' + (MPI_OPTIONS).toStdString() + ' ' + qtutils::addDoubleQuotes(CTC_BIN).toStdString() +
					" -merge " + " -project " + qtutils::addDoubleQuotes(ctcFilenameWithPath).toStdString() + (" -save ") + saveP3
					),
				true
			)
		);

		log("- The CWD is: " + process.workingDirectory());

#ifdef FUTURE_USE
		// processCommand does not work for //bsub/sbatch <myJobs.sh, because i/o redirection is handled by a shell, not by QProcess or the external exe
		auto isOk = processShCommand(process, runCTC);
#else
		auto isOk = processCommand(process, runCTC);
#endif

		if (isOk)
			log("- Wait for CTC/PWD Computations...");
		loop.exec(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);

	}

	QString Controller::createCTCscenarioFolder(const QString& filePath) const
	{
		//assume the directory exists and contains some files and you want all project3d and PROJECT3D files
		QDir directory(QFileInfo(filePath).absoluteDir().path());
		QDir baseDir(directory);
		QStringList project3d = directory.entryList(QStringList() << "*.project3d" << "*.PROJECT3D", QDir::Files);
		foreach(QString filename, project3d) {
			//do whatever you need to do
			QFileInfo fInfo(filename);
			if (fInfo.baseName().contains("CTC")) {
				baseDir.cdUp();
				break;
			}
		}

		QString scenarioFolderPath;
		for (int i = 0; i < MaxDirCount; ++i)
		{
			QString fldrName = "/" + QString::number(i + 1);
			scenarioFolderPath = baseDir.path().append(fldrName);
			QFileInfo newFldr(scenarioFolderPath);
			if (!newFldr.exists())
				break;
		}
		log("- Newly created Scenario folder: " + scenarioFolderPath);

		QTimer timer;
		timer.start(100);
		while (timer.remainingTime() > 0)
		{
			qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
		}


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

		return scenarioFolderPath;
	}

	void ctcWizard::Controller::finalizeProject3dFile(const QString& oldp3file, const QString& newp3file) const
	{
		QFile oldFile(oldp3file);
		QFile newFile(newp3file);

		QFileInfo info(newp3file);
		if (info.exists()) newFile.remove();

		oldFile.open(QIODevice::ReadOnly | QIODevice::Text);
		newFile.open(QIODevice::ReadWrite | QIODevice::Text);

		QTextStream oldStream(&oldFile);
		QTextStream newStream(&newFile);
		QString line = oldStream.readLine();
		newStream << line << endl;


		while (!line.isNull()) {
			line = oldStream.readLine();
			newStream << line << endl;
			//        
			//[GridMapIoTbl]
			if (line.contains("[GridMapIoTbl]", Qt::CaseSensitive)) {
				line = oldStream.readLine();
				newStream << line << endl;
				line = oldStream.readLine();
				newStream << line << endl;
				line = oldStream.readLine();
				newStream << line << endl;

				line = oldStream.readLine();
				newStream << line << endl;
				while (!line.contains("[End]", Qt::CaseSensitive)) {
					line = oldStream.readLine();
					QStringList theLines = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
					if (theLines[0].compare("\"CTCRiftingHistoryIoTbl\""))
						newStream << line << endl;
				}
			}

			//[CTCRiftingHistoryIoTbl]
			if (line.contains("[CTCRiftingHistoryIoTbl]", Qt::CaseSensitive)) {
				line = oldStream.readLine();
				newStream << line << endl;
				line = oldStream.readLine();
				newStream << line << endl;
				line = oldStream.readLine();
				newStream << line << endl;

				line = oldStream.readLine();
				newStream << line << endl;
				while (!line.contains("[End]", Qt::CaseSensitive)) {
					line = oldStream.readLine();

					QRegExp re("\"([A-Za-z0-9_\\./\\-\\s]*)\"");

					QStringList list;
					int pos = 0;

					while ((pos = re.indexIn(line, pos)) != -1) {
						list << re.cap(1);
						pos += re.matchedLength();
					}

					if (list.size() > 1)
					{
						if (list[1].compare("")) {
							line.replace(list[1], QString("")); // replace text in string
#ifdef FUTURE_USE
							qDebug() << line;
#endif
						}
						if (list[2].compare(""))
						{
							line.replace(list[2], QString("")); // replace text in string
#ifdef FUTURE_USE
							qDebug() << line;
#endif
						}
					}
					newStream << line << endl;
				}
			}
		}
		oldFile.close();
		newFile.close();
	}

	// This function will delete the CTC scenario folder from which scenario for ALC has been created
	void Controller::deleteCTCscenario(const QString& scenarioFolder) const
	{
		QTimer timer;
		timer.start(100);
		while (timer.remainingTime() > 0)
		{
			qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
		}

		QProcess process;
		connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });


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

	bool Controller::createScenarioForALC(const QString& scenarioFolder) const
	{
		QString prefix_ = "CTCv2-";
		QString folderNameForALCscenario = "ALC_Scenario_" + qtutils::getTimeStamp(prefix_);
		log("- folderNameForALCscenario: " + folderNameForALCscenario);

		QDir newDir(scenarioFolder);
		newDir.mkdir(folderNameForALCscenario);

		QTimer timer;
		timer.start(100);
		while (timer.remainingTime() > 0)
		{
			qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
		}

		QProcess process;
		connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });
		process.setWorkingDirectory(scenarioFolder);

		QDir scenarioDirec(scenarioFolder);
		QStringList fileList1 = scenarioDirec.entryList(QStringList() << "*.*", QDir::Files);

		foreach(QString file, fileList1) {
			QFileInfo fileInfo(file);
			auto baseName = fileInfo.baseName();
			auto extension = fileInfo.completeSuffix();
			if (baseName.contains("_OUT", Qt::CaseInsensitive) && !extension.compare("project3d", Qt::CaseInsensitive))
			{
				QString source = scenarioFolder + "/" + file;
				// always name the zipped p3 file as Project.project3d, import will complain otherwise 
				QString target = scenarioFolder + "/" + folderNameForALCscenario + "/" + "Project.project3d";
				finalizeProject3dFile(source, target);
			}
			else if (!file.compare("Inputs.HDF", Qt::CaseInsensitive)
				|| !fileInfo.completeSuffix().compare("txt", Qt::CaseInsensitive)
				|| !fileInfo.completeSuffix().compare("FLT", Qt::CaseInsensitive))
			{
				QFile::copy(scenarioFolder + "/" + file, scenarioFolder + "/" + folderNameForALCscenario + "/" + file);
			}
		}

		QDir cpyScenarioDirec(scenarioFolder + "/" + folderNameForALCscenario);

		auto cwd = cpyScenarioDirec.path();
		process.setWorkingDirectory(cwd);

		QStringList fileList2 = cpyScenarioDirec.entryList(QStringList() << "*.*", QDir::Files);

		bool success = false;
#ifdef Q_OS_WIN
		foreach(QString file, fileList2) {
			success = processCommand(process, QString("zip " + folderNameForALCscenario + ".zip " + file));

#else
		foreach(QString file, fileList2) {
			success = processCommand(process, QString("zip -u " + folderNameForALCscenario + ".zip " + file));
#endif
			process.waitForFinished(-1);
		}

		// ------------- Fixed --------------//
		QStringList fileList3 = cpyScenarioDirec.entryList(QStringList() << "*.zip", QDir::Files);

		for (int i = 0; i < fileList3.count(); i++)
		{
			auto origPath = cwd + "/" + fileList3[i];
			auto destPath = scenarioFolder + "/../" + fileList3[i];
			success = QFile::rename(origPath, destPath);

		}

		if (success) {

			log("- Scenario for ALC workflow \"" + folderNameForALCscenario + ".zip\" successfully created!!!");
		}
		else {
			log("- Scenario for ALC workflow \"" + folderNameForALCscenario + ".zip\" not created, something went wrong!");
		}

		return success;
	}

	void Controller::launchCauldronMapsTool(const QString & filePath) const
	{
		log("- Launching cauldronmaps tool to view CTC output maps");

#ifdef Q_OS_UNIX
		QProcess myProcs;
		connect(&myProcs, &QProcess::readyReadStandardOutput, [&]() {log(QString(myProcs.readAllStandardOutput())); });
		processCommand(myProcs, QString("which cauldronmaps"));
#endif
		QFileInfo info(filePath);
		QTimer timer;
		timer.start(100);
		while (timer.remainingTime() > 0)
		{
			qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
		}

		QProcess process;
		connect(&process, &QProcess::readyReadStandardOutput, [&]() {log(QString(process.readAllStandardOutput())); });
		process.setWorkingDirectory(info.absoluteDir().path());
#ifdef Q_OS_UNIX
		processCommand(process, QString("cauldronmaps"));
#else
		log("cauldronmaps is a LINUX application only, sorry!!");
#endif
		process.waitForFinished(-1);
	}


	void Controller::mapOutputCtcScript(const QString & filePath) const
	{
		QFileInfo info(filePath);
		auto cwd = info.absoluteDir().path();
		QStringList strList = info.fileName().simplified().split(".");
		if (QFileInfo(cwd + "/" + strList[0] + "_OUT.project3d").exists())
		{
			QTimer timer;
			timer.start(100);
			while (timer.remainingTime() > 0)
			{
				qApp->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
			}
			QString ctcDirName = strList[0] + "_OUT_CauldronOutputDir";
			QDir sourceDir(cwd + "/" + strList[0] + "_CauldronOutputDir");
			QDir destDir(cwd + "/" + ctcDirName);
			if (!destDir.exists()) {
				if (!makeDirSymLinks(sourceDir, destDir)) qDebug() << "Can't create symlink";
			}
			else {
				destDir.removeRecursively();
				QString name = destDir.absolutePath();
				log("deleted " + name);
				if (!makeDirSymLinks(sourceDir, destDir)) qDebug() << "Can't create symlink";
			}
			log("- CTC Scenario: " + info.absoluteDir().path());
			log("- Successfully Completed!!!");
			log("- To visualize CTC output use the button: View CTC Output Maps");
			log("- And select project3d file: \"" + strList[0] + "_OUT.project3d\" from \"" + info.absoluteDir().path() + "\" to view CTC output maps");
		}
		else
			log("- CTC run not Successfully Completed!!!");
	}

	bool Controller::processCommand(QProcess & process, const QString & command) const
	{
		log("- Executing: " + command);
		if (process.state() == QProcess::NotRunning)//Its not running if this is true
		{
#ifdef Q_OS_WIN
			process.start(command);
#else
			process.start("sh", QStringList() << "-c" << command);
#endif // Q_OS_WIN
		}

		if (!process.waitForStarted()) //default wait time 30 sec
		{
			process.kill();
			log("- Process stopped, did not start within 30 seconds");
			return false;
		}
		auto exitCode = process.exitCode();
#ifdef FUTURE_USE
		auto exitStatus = process.exitStatus();
		qDebug() << "Exit code:" << exitCode << ' ' << exitStatus;
#endif
		return !exitCode;
		}

	bool ctcWizard::Controller::makeDirSymLinks(const QDir & src, const QDir & desti) const
	{
		QFile dir(src.absolutePath());
		bool ok = dir.link(desti.absolutePath());

		return ok;
	}

	void Controller::log(const QString & text) const
	{
		ui_.lineEditLog()->append(text);
		ui_.lineEditLog()->verticalScrollBar()->setSliderPosition(
			ui_.lineEditLog()->verticalScrollBar()->maximum());
	}

	void Controller::slotPushClearLogClicked() const
	{
		ui_.lineEditLog()->setText("");
	}

	void ctcWizard::Controller::processError(QProcess::ProcessError error) const
	{
		qWarning() << "error in process! " << error;
	}

	void ctcWizard::Controller::processFinished(int exitCode, QProcess::ExitStatus exitStatus, const qtutils::FileLogger & fl,
		const QString & logFilePath, QTextEdit * theText, workloadmanagers::WorkLoadManagerType wlmType) const
	{
		qDebug() << "WARNING: is the run LOCAL? " << (wlmType == workloadmanagers::WorkLoadManagerType::LOCAL);
#ifdef Q_OS_UNIX
		if (wlmType != workloadmanagers::WorkLoadManagerType::LOCAL) {
			fl.finishupLogging(logFilePath, theText);
		}
#else
		fl.finishupLogging(logFilePath, theText);
#endif
		QString theBig = QVariant::fromValue(exitStatus).toString();
		QString status = exitCode != 0 ? "Processing crashed" : "Processing completed successfully";
		/// when it Crashes, the error code should help us
		log(status + "! with " + theBig + " status & error code, " + QVariant::fromValue(exitCode).toString() + ".");
		log("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
	}

	} // namespace ctcWizard
