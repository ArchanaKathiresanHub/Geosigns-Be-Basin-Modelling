#include <PasswordDialog.h>
#include <GlobalSettings.h>

PasswordDialog::PasswordDialog(QWidget *parent , Qt::WindowFlags f )
: QDialog(parent,f)
{
	setupUi(this);

	RetrieveFromSettings();
}

void PasswordDialog::accept()
{
	QDialog::accept();

	SetSettings();
}

void PasswordDialog::RetrieveFromSettings()
{
	_usernameLineEdit->setText(GlobalSettings::GetUsername());
	_passwordLineEdit->setText(GlobalSettings::GetPassword());
}

void PasswordDialog::SetSettings()
{
	GlobalSettings::SetUsername(_usernameLineEdit->text());
	GlobalSettings::SetPassword(_passwordLineEdit->text());
}
