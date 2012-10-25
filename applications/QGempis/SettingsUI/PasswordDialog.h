#include <ui_PasswordDialog.h>

class PasswordDialog : public QDialog , public Ui::PasswordDialog
{
public:
	PasswordDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

	virtual void accept();

protected:
	void RetrieveFromSettings();
	void SetSettings();
};
