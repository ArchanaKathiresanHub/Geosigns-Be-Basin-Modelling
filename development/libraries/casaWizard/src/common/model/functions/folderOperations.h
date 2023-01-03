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

QString exportScenarioToZip(const QDir& sourceDir, const QString& workingDirectory, const QString& projectFile, InfoGenerator& infoGenerator);

void processCommand(QProcess& process, const QString& command);

void cleanFolder(const QDir& folder);

} // namespace functions

} // namespace casaWizard
