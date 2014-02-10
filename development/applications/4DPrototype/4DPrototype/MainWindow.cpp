#include "MainWindow.h"

MainWindow::MainWindow()
{
	m_ui.setupUi(this);

	// Remove all the ugly buttons and scroll wheels that 
	// you always get for free with these OIV viewers
	m_ui.widget->setDecoration(false);
}