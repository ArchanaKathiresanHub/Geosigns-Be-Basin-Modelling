// Widget that contains a row number incorporated in the pushbutton. This is row number is necessary to connect it to other events.
// This could be done with the new style connect with functors in Qt. This however does not work under all conditions with the Intel compiler
#pragma once

#include <QPushButton>

namespace casaWizard
{

class TableRowPushButton : public QPushButton
{
  Q_OBJECT

public:
  explicit TableRowPushButton(const QString& text, int tableRow, QWidget* parent = 0);

signals:
  void clicked(const int row);

private slots:
  void handleClicked();

private:
  const int tableRow_;
};

} // namespace casaWizard
