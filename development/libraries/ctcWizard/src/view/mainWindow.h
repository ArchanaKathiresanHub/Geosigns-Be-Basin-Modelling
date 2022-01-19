#pragma once

#include <QMainWindow>

class QAction;
class QTabWidget;
class QTextEdit;
class QPushButton;

namespace ctcWizard
{
	constexpr const char* CTCUI_VERSION = " 1.7.2";
	class CTCtab;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		MainWindow(QWidget* parent = 0);

		void setupUi();

		const QAction* actionExit() const;
		const QAction* actionOpenFile() const;

		const QPushButton* pushClearLog() const;
		QTextEdit* lineEditLog() const;
		const QTabWidget* tabWidget() const;

		CTCtab* ctcTab() const;

	private slots:
		void slotLineEditProject3dTextChanged(const QString& project3dPath);

	private:
		QString project3dPath_;
		QAction* actionExit_;
		QAction* actionOpenFile_;

		QPushButton* pushClearLog_;
		QTextEdit* lineEditLog_;
		QTabWidget* tabWidget_;

		CTCtab* ctcTab_;
		void closeEvent(QCloseEvent* event);
	};

} // namespace ctcWizard
