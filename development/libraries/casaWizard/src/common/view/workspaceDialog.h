// Class responsible to create a dialogue in which the user will be able to choose between different options for the workspace location.
#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QFormLayout;
class QRadioButton;
class QStringList;

namespace casaWizard
{

class WorkspaceDialog : public QDialog
{
  Q_OBJECT

public:
  explicit WorkspaceDialog(const QString originalPath, const QString sugestedPath, const QString& windowTitle = "", QWidget* parent = 0);

  QString optionSelected() const;
  QString userWorkspaceLocation() const;

  void addOption(const QString& variableName, const double value = 0);
  void addComboBox(const QString& variableName, const QStringList& options, const QString& selectedOption = "");

public slots:
  void accept();

private:
  QString optionSelected_;

  QFormLayout* formLayout_;

  QRadioButton *radio1_;
  QRadioButton *radio2_;
  QRadioButton *radio3_;
};

} // namespace casaWizard
