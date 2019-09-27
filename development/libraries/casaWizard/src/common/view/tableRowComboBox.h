// Widget that contains a row number incorporated in the combobox. This is row number is necessary to connect it to other events.
// This could be done with the new style connect with functors in Qt. This however does not work under all conditions with the Intel compiler
#pragma once

#include <QComboBox>

namespace casaWizard
{

class TableRowComboBox : public QComboBox
{
  Q_OBJECT

public:
  explicit TableRowComboBox(int tableRow, QWidget* parent = 0);

signals:
  void currentTextChanged(const int row, const QString& currentText);
  void currentIndexChanged(const int row, const int selectedIndex);

private slots:
  void handleTextChanged(QString currentText);
  void handleIndexChanged(int selectedIndex);

private:
  const int tableRow_;
};

} // namespace casaWizard
