#ifndef _WAIT_WIDGET_H_
#define _WAIT_WIDGET_H_

#include <QWidget>
#include <ui_WaitWidget.h>
#include <QTimer>

class MainWindow;

class WaitWidget : public QWidget, public Ui::WaitWidget
{
	Q_OBJECT
public:
	WaitWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	void Show(MainWindow* mainWindow);
	void Hide();

protected slots:
	void TimerSlot();
	void CancelSlot();

protected:
	QTimer _timer;
	MainWindow* _mainWindow;
};

#endif
