// General function to copy a case folder
#pragma once

#include <QDir>

class QProcess;

namespace casaWizard
{

class InfoGenerator;

namespace functions
{

bool copyCaseFolder(const QDir sourceDir, const QDir targetDir);

bool overwriteIfDirectoryExists(const QString& directory);

void zipFolderContent(const QDir& sourceDir, const QString &targetDestination, const QString &zipName);

void exportScenarioToZip(const QDir& sourceDir, const QString& workingDirectory, const QString& projectFile, InfoGenerator& infoGenerator);

void processCommand(QProcess& process, const QString& command);

void cleanFolder(const QDir& folder, const QString& projectFile);

} // namespace functions

} // namespace casaWizard
