#include "Qt_Utils.h"

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

