#include "textEditDialog.h"

#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>

namespace casaWizard
{

TextEditDialog::TextEditDialog(const QString& text, QWidget* parent) :
  QDialog(parent),
  lineEdit_{new QTextEdit(this)}
{
  lineEdit_->setPlainText(text);
  QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->addWidget(lineEdit_);
  layout->addWidget(buttonBox);
}

QString TextEditDialog::plainText() const
{
  return lineEdit_->toPlainText();
}

}  // namespace casaWizard
