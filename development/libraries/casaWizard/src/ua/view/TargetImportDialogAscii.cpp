#include "TargetImportDialogAscii.h"
#include "TargetImportLayout.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetImportDialogAscii::TargetImportDialogAscii(QWidget* parent):
   QDialog(parent),
   m_pushSelectFile{new QPushButton("Select", parent)},
   m_lineEditFile{new QLineEdit(parent)},
   m_targetImportLayout{new TargetImportLayout()},
   m_lineEditName{new QLineEdit(this)}
{
   setWindowTitle("Import options");

   QHBoxLayout* layoutFileSelect = new QHBoxLayout();
   layoutFileSelect->addWidget(new QLabel("File", parent));
   layoutFileSelect->addWidget(m_lineEditFile);
   layoutFileSelect->addWidget(m_pushSelectFile);

   QHBoxLayout* layoutName = new QHBoxLayout();
   layoutName->addWidget(new QLabel("Name", this));
   layoutName->addStretch(1);
   layoutName->addWidget(m_lineEditName);

   QVBoxLayout* layout = new QVBoxLayout(this);
   layout->addLayout(layoutFileSelect);
   layout->addLayout(m_targetImportLayout);
   layout->addLayout(layoutName);
}

} // namespace ua

} // namespace casaWizard
