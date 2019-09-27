#include "formDialog.h"

#include <QString>
#include <QDialogButtonBox>
#include <QLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QStringList>

namespace casaWizard
{

FormDialog::FormDialog(const QString& windowTitle, QWidget* parent) :
  QDialog(parent),
  optionValues_{},
  comboBoxValues_{},
  lineEdits_{},
  comboBoxes_{},
  formLayout_{}
{
  QVBoxLayout* total = new QVBoxLayout();
  formLayout_ = new QFormLayout();

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  total->addLayout(formLayout_);
  total->addWidget(buttons);

  setLayout(total);

  if (!windowTitle.isEmpty())
  {
    setWindowTitle(windowTitle);
  }
}

void FormDialog::accept()
{
  optionValues_.clear();
  comboBoxValues_.clear();
  for (const QLineEdit* lineEdit : lineEdits_)
  {
    optionValues_.append(lineEdit->text().toDouble());
  }

  for (const QComboBox* comboBox: comboBoxes_)
  {
    comboBoxValues_.append(comboBox->currentText());
  }
  QDialog::accept();
}

QStringList FormDialog::comboBoxValues() const
{
  return comboBoxValues_;
}

QVector<double> FormDialog::optionValues() const
{
  return optionValues_;
}

void FormDialog::addOption(const QString& variableName, const double value)
{  
  QLineEdit* newLineEdit = new QLineEdit(this);
  newLineEdit->setText(QString::number(value));
  newLineEdit->setValidator(new QDoubleValidator(newLineEdit));

  lineEdits_.push_back(newLineEdit);
  formLayout_->addRow(variableName, newLineEdit);
}

void FormDialog::addComboBox(const QString& variableName, const QStringList& options, const QString& selectedOption)
{
  QComboBox* newComboBox = new QComboBox(this);
  int selectedIndex = options.indexOf(selectedOption);
  selectedIndex = (selectedIndex == -1)?0:selectedIndex;
  newComboBox->addItems(options);
  newComboBox->setCurrentIndex(selectedIndex);

  comboBoxes_.push_back(newComboBox);
  formLayout_->addRow(variableName, newComboBox);
}

} // namespace casaWizard
