#include "helpLabel.h"

casaWizard::HelpLabel::HelpLabel(QWidget* parent, const QString& tooltipText) :
  QLabel(parent)
{
  QPixmap helpPixmap = QPixmap(":/Help.png").scaledToHeight(20, Qt::TransformationMode::SmoothTransformation);
  setPixmap(helpPixmap);
  setFixedWidth(helpPixmap.width());
  setToolTip(tooltipText);
}

void casaWizard::HelpLabel::setSize(int pts)
{
   QPixmap helpPixmap = QPixmap(":/Help.png").scaledToHeight(pts, Qt::TransformationMode::SmoothTransformation);
   setPixmap(helpPixmap);
   setFixedWidth(helpPixmap.width());
}
