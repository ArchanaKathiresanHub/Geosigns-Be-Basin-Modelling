// Dynamic dialog window with line entries and comboboxes
#pragma once

#include <QVector>
#include <QDialog>

class QLineEdit;
class QComboBox;
class QStringList;
class QFormLayout;

namespace casaWizard
{

class FormDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FormDialog(const QString& windowTitle = "", QWidget* parent = 0);

  QVector<double> optionValues() const;
  QStringList comboBoxValues() const;

  void addOption(const QString& variableName, const double value = 0);
  void addComboBox(const QString& variableName, const QStringList& options, const QString& selectedOption = "");

public slots:
  void accept();

private:
  QVector<double> optionValues_;
  QStringList comboBoxValues_;

  QVector<QLineEdit*> lineEdits_;
  QVector<QComboBox*> comboBoxes_;

  QFormLayout* formLayout_;
};

} // namespace casaWizard
