#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "MainWindow.ui.h"

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT;

	Ui::MainWindow m_ui;

  void connectSignals();

private slots:

  void onActionOpenTriggered();

  void onSliderValueChanged(int value);

public:

	MainWindow();
};

#endif
