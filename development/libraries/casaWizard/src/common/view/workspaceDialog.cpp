#include "workspaceDialog.h"

#include "view/sharedComponents/customradiobutton.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QString>
#include <QStringList>

namespace casaWizard
{

WorkspaceDialog::WorkspaceDialog(const QString originalPath, const QString suggestedPath, const QString& windowTitle, QWidget* parent) :
  QDialog(parent),
  optionSelected_{},
  formLayout_{},
  radio1_{},
  radio2_{}
{
  QVBoxLayout* total = new QVBoxLayout();
  formLayout_ = new QFormLayout();

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  total->addLayout(formLayout_);

  radio1_ = new CustomRadioButton("Option 1.- Suggested working directory: \n\n " + suggestedPath + "\n");
  radio1_->setProperty("id",suggestedPath);
  radio2_ = new CustomRadioButton("Option 2.- Current working directory: \n\n " + originalPath + "\n");
  radio2_->setProperty("id", originalPath);

  radio1_->setChecked(true);

  total->addWidget(radio1_);
  total->addWidget(radio2_);
  total->addStretch(1);

  total->addWidget(buttons);
  setLayout(total);

  if (!windowTitle.isEmpty())
  {
    setWindowTitle(windowTitle);
  }

  optionSelected_="";
}

void WorkspaceDialog::accept()
{

  QDialog::accept();

  if (radio1_->isChecked())
  {
    optionSelected_ = radio1_->property("id").toString();
  }
  else if (radio2_->isChecked())
  {
    optionSelected_ = radio2_->property("id").toString();
  }
}

QString WorkspaceDialog::optionSelected() const
{
  return optionSelected_;
}

} // namespace casaWizard
