#include <ui_MainWindow.h>
#include <BackgroundSSHSession.h>
#include <QTimer>
#include <WaitWidget.h>

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	void CancelPressed();

protected slots:
	void ChangePasswordSlot();
	void JobSubmitSlot();
	void TimerSlot();

protected:
	QTimer _timer;
	BackgroundSSHSession* _backgroundSSHSession;
	WaitWidget _waitWidget;
};
