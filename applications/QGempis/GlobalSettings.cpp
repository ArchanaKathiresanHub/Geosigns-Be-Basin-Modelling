#include <GlobalSettings.h>
#include <QSettings>
#include <MyDefines.h>

QString GlobalSettings::GetUsername()
{
	QSettings settings(ORGANISATION_NAME,APPLICATION_NAME);

	QString username=settings.value("username").toString();

	return username;
}

void GlobalSettings::SetUsername(const QString& s)
{
	QSettings settings(ORGANISATION_NAME,APPLICATION_NAME);

	settings.setValue("username",s);
}

QString GlobalSettings::GetPassword()
{
	QSettings settings(ORGANISATION_NAME,APPLICATION_NAME);

	QString password=settings.value("password").toString();

	return password;
}

void GlobalSettings::SetPassword(const QString& s)
{
	QSettings settings(ORGANISATION_NAME,APPLICATION_NAME);

	settings.setValue("password",s);
}
