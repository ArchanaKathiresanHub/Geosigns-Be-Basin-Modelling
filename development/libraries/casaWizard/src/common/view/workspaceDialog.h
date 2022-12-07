// Class responsible to create a dialogue in which the user will be able to choose between different options for the workspace location.
#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QFormLayout;
class CustomRadioButton;
class QStringList;

namespace casaWizard
{

class WorkspaceDialog : public QDialog
{
  Q_OBJECT

public:
  WorkspaceDialog(const QString originalPath, const QString suggestedPath, const QString& windowTitle = "Select workspace", QWidget* parent = nullptr);

  QString optionSelected() const;

  void addOption(const QString& variableName, const double value = 0);
  void addComboBox(const QString& variableName, const QStringList& options, const QString& selectedOption = "");

public slots:
  void accept();

private:
  QString optionSelected_;

  QFormLayout* formLayout_;

  CustomRadioButton *radio1_;
  CustomRadioButton *radio2_;
};

} // namespace casaWizard
