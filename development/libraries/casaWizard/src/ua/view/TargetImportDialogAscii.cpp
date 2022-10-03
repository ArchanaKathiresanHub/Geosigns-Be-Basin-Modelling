#include "TargetImportDialogAscii.h"
#include "TargetImportLayout.h"

#include "view/sharedComponents/helpLabel.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

namespace casaWizard
{

namespace ua
{

TargetImportDialogAscii::TargetImportDialogAscii(QWidget* parent):
   QDialog(parent),
   m_pushSelectFile{new QPushButton("Select", parent)},
   m_lineEditFile{new QLineEdit(parent)},
   m_targetImportLayout{new TargetImportLayout()},
   m_lineEditName{new QLineEdit(this)},
   m_helpLabelName{new HelpLabel(this, "Set a default name for unnamed locations. Locations will be named as: <Name>_n")}
{
   setWindowTitle("Import options");
   m_helpLabelName->setSize(10);
   //upper line
   QHBoxLayout* layoutFileSelect = new QHBoxLayout();
   layoutFileSelect->addWidget(new QLabel("File", parent));
   layoutFileSelect->addWidget(m_lineEditFile);
   layoutFileSelect->addWidget(m_pushSelectFile);

   //bottom line
   QHBoxLayout* layoutName = new QHBoxLayout();

   m_lineEditName->setText("PSW");

   layoutName->addWidget(new QLabel("Name", this));
   layoutName->addStretch(1);
   layoutName->addWidget(m_lineEditName);
   layoutName->addWidget(m_helpLabelName);

   //Ok | Cancel widget
   QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
   connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
   connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

   //whole overlay
   QVBoxLayout* layout = new QVBoxLayout(this);
   layout->addLayout(layoutFileSelect);
   layout->addLayout(m_targetImportLayout);
   layout->addLayout(layoutName);
   layout->addWidget(buttons);
}

QLineEdit* TargetImportDialogAscii::lineEditFile() const
{
   return m_lineEditFile;
}

QLineEdit* TargetImportDialogAscii::lineEditName() const
{
   return m_lineEditName;
}

const QPushButton* TargetImportDialogAscii::pushSelectFile() const
{
   return m_pushSelectFile;
}

QVector<bool> TargetImportDialogAscii::surfaceSelectionStates() const
{
   return m_targetImportLayout->surfaceSelectionStates();
}

bool TargetImportDialogAscii::temperatureTargetsSelected() const
{
   return m_targetImportLayout->temperatureTargetsSelected();
}

bool TargetImportDialogAscii::vreTargetsSelected() const
{
   return m_targetImportLayout->vreTargetsSelected();
}

QString TargetImportDialogAscii::depthInput() const
{
   return m_targetImportLayout->depthInput();
}

void TargetImportDialogAscii::updateSurfaceTable(const QStringList& surfaces)
{
   m_targetImportLayout->updateSurfaceTable(surfaces);
}

} // namespace ua

} // namespace casaWizard
