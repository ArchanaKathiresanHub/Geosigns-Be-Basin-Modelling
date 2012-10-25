#include <WaitWidget.h>
#include <QProgressBar>
#include <MainWindow.h>

WaitWidget::WaitWidget(QWidget *parent , Qt::WindowFlags flags )
: QWidget(parent,flags)
{
	setupUi(this);
	QObject::connect(&_timer,SIGNAL(timeout()),this,SLOT(TimerSlot()));
	QObject::connect(_pushButton,SIGNAL(released()),this,SLOT(CancelSlot()));

	_progressBar->setRange(0,100);
}

void WaitWidget::Show(MainWindow* mainWindow)
{
	_mainWindow=mainWindow;

	_progressBar->setValue(0);
	_timer.start(1000);
	show();
}

void WaitWidget::Hide()
{
	_timer.stop();
	hide();
}

void WaitWidget::TimerSlot()
{
	int value=_progressBar->value();
	if (value>=100)
		value=0;
	else
		value++;

	_progressBar->setValue(value);
}

void WaitWidget::CancelSlot()
{
	_mainWindow->CancelPressed();
}
