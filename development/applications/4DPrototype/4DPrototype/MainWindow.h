#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "MainWindow.ui.h"

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
	Q_OBJECT;

	Ui::MainWindow m_ui;

public:

	MainWindow();
};

#endif
