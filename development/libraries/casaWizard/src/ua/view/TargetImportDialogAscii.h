#pragma once

#include <QDialog>
#include <QLineEdit>

namespace casaWizard
{

class CustomCheckbox;

namespace ua
{

class TargetImportLayout;

class TargetImportDialogAscii : public QDialog
{
   Q_OBJECT

public:
   TargetImportDialogAscii(QWidget* parent = nullptr);

   const QPushButton* pushSelectFile() const;
   QLineEdit* lineEditFile() const;
   QLineEdit* lineEditName() const;

   QVector<bool> surfaceSelectionStates() const;
   bool temperatureTargetsSelected() const;
   bool vreTargetsSelected() const;
   QString depthInput() const;

   void updateSurfaceTable(const QStringList& surfaces);
private:

   QPushButton* m_pushSelectFile;
   QLineEdit* m_lineEditFile;
   TargetImportLayout* m_targetImportLayout;
   QLineEdit* m_lineEditName;
};

} // namespace ua

} // namespace casaWizard

