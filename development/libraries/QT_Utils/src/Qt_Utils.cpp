#include "Qt_Utils.h"
#include <QCoreApplication>

QString qtutils::getTimeStamp(const QString& prefix)
{
	const  QDateTime now{ QDateTime::currentDateTime() };
	return prefix + now.toString("yyyyMMdd-HHmmss");
}

QTextStream& qtutils::qStdOut()
{
	static QTextStream ts(stdout);
	return ts;
}

QString qtutils::AddDoubleQuotes(QString value)
{
	return "\"" + value + "\"";
}

std::string replaceFirstOccurrence(
	std::string& s,
	const std::string& toReplace,
	const std::string& replaceWith)
{
	std::size_t pos = s.find(toReplace);
	if (pos == std::string::npos) return s;
	return s.replace(pos, toReplace.length(), replaceWith);
}

QString qtutils::ExportApplicationPath(void)
{
	std::string applicationPath = QCoreApplication::applicationDirPath().toStdString();
	std::size_t index = applicationPath.find("/apps/sss");
	if (index != std::string::npos)
	{
		applicationPath = applicationPath.substr(index);
		// there is superfluous name extension in AMS ssstest. 
		index = applicationPath.find("ssstest_rhn");
		if (index != std::string::npos)
		{
			replaceFirstOccurrence(applicationPath, "ssstest_rhn", "ssstest");
		}
	}
	return QString::fromStdString(applicationPath);
}

QString qtutils::IsValidNoOfProcs(QString noOfProcs)
{
	bool validate;
	auto value = noOfProcs.toInt(&validate);
	QString ErrMsg = "Ok";
	ErrMsg = !validate ? " no. of Processors should be an Integer!" : ErrMsg ;
	ErrMsg = value < 0  ? " no. of Processors should be a positive Integer!" : ErrMsg;
	ErrMsg = value > MAX_PROCS ? " no. of Processors should be < " + QString::number(MAX_PROCS) + "!" : ErrMsg;

	return ErrMsg;
}
