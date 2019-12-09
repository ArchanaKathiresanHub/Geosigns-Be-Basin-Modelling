#include "workspaceDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QRadioButton>
#include <QString>
#include <QStringList>

namespace casaWizard
{

WorkspaceDialog::WorkspaceDialog(const QString originalPath, const QString suggestedPath, const QString& windowTitle, QWidget* parent) :
  QDialog(parent),
  optionSelected_{},
  formLayout_{},
  radio1_{},
  radio2_{},
  radio3_{}
{
  QVBoxLayout* total = new QVBoxLayout();
  formLayout_ = new QFormLayout();

  QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

  total->addLayout(formLayout_);

  radio1_ = new QRadioButton("Option 1.- Suggested working directory: \n\n " + suggestedPath + "\n");
  radio1_->setProperty("id",suggestedPath);
  radio2_ = new QRadioButton("Option 2.- Current working directory: \n\n " + originalPath + "\n");
  radio2_->setProperty("id", originalPath);
  radio3_ = new QRadioButton("Option 3.- Select new directory");
  radio3_->setProperty("id", "");

  radio1_->setChecked(true);

  total->addWidget(radio1_);
  total->addWidget(radio2_);
  total->addWidget(radio3_);
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
  else
  {
    optionSelected_ = userWorkspaceLocation();
  }
}

QString WorkspaceDialog::optionSelected() const
{
  return optionSelected_;
}

QString WorkspaceDialog::userWorkspaceLocation() const
{
  const QDir casaCaseDir(QFileDialog::getExistingDirectory(0, "Save CASA case to directory","",
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

  return casaCaseDir.absolutePath();
}

} // namespace casaWizard
