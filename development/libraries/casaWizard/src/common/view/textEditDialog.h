// Dialog to edit text
#pragma once

#include <QDialog>

class QString;
class QTextEdit;

namespace casaWizard
{

class TextEditDialog: public QDialog
{
  Q_OBJECT

public:
  explicit TextEditDialog(const QString& text, QWidget* parent = 0);
  QString plainText() const;

private:
  QTextEdit* lineEdit_;

};

}  // namespace casaWizard
