//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "importWellPopupVSET.h"

#include "view/components/customComboBox.h"
#include "view/components/ErrorMessageBox.h"
#include "model/logger.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedLayout>
#include <QTableWidget>
#include <QVBoxLayout>

#include <fstream>
#include <sstream>

namespace casaWizard
{

ImportWellPopupVSET::ImportWellPopupVSET(QWidget *parent) :
   ImportWellPopup(parent),
   filterOptions_(new QComboBox(this)),
   filterLayout_(new QStackedLayout()),
   selectedFileLabel_(nullptr)
{
   setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

   filterOptions_->addItems({"None", "Every x vset entry", "Minimum pseudo well extraction distance", "Get well locations from file"});
   connect(filterOptions_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotHandleFilterOptionChanged(int)));
   filterLayout_->addWidget(new QWidget(this));
   filterLayout_->addWidget(createSkipWidget());
   filterLayout_->addWidget(createDistanceWidget());
   filterLayout_->addWidget(createWellsFileWidget());

   QHBoxLayout* nameLayout = new QHBoxLayout();
   nameLayout->addWidget(new QLabel("Pseudo well identifier: ", this),0);
   QLabel* wellNameLabel = new QLabel("", this);
   nameLayout->addWidget(createWellIdentifierWidget(wellNameLabel),0);
   nameLayout->addWidget(wellNameLabel,1);

   QHBoxLayout* filterLayout = new QHBoxLayout();
   filterLayout->addWidget(new QLabel("Filter: ", this));
   filterLayout->addWidget(filterOptions_);
   filterLayout->addLayout(filterLayout_);

   QVBoxLayout* layout = new QVBoxLayout(this);
   layout->addLayout(nameLayout, 1);
   layout->addLayout(filterLayout);
   layout->addWidget(propertyMappingTable_);
   layout->addWidget(buttons_);

   propertyMappingTable_->setFixedHeight(80);
}

void ImportWellPopupVSET::updateTableUsingOptions(const ImportOptionsVSET& importOptions)
{
   importOptions_ = importOptions;

   propertyMappingTable_->clearContents();
   propertyMappingTable_->setColumnCount(2);
   propertyMappingTable_->setHorizontalHeaderItem(0, new QTableWidgetItem("Vertical domain"));
   propertyMappingTable_->setHorizontalHeaderItem(1, new QTableWidgetItem("Velocity"));
   propertyMappingTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

   propertyMappingTable_->setRowCount(1);
   propertyMappingTable_->setItem(0, 1, new QTableWidgetItem("Vint"));

   QComboBox* propertySelector = new CustomComboBox();

   propertySelector->addItems({"Depth", "TwoWayTime"});
   propertySelector->setCurrentText({"Depth"});
   propertyMappingTable_->setCellWidget(0, 0, propertySelector);

   update();
}

QLineEdit* ImportWellPopupVSET::createWellIdentifierWidget(QLabel* wellNameLabel)
{
   QLineEdit* wellIdentifierNameWidget = new QLineEdit(importOptions_.wellIdentifierName);
   auto updateName = [=](){ importOptions_.wellIdentifierName = wellIdentifierNameWidget->text();
      wellNameLabel->setText("Name well N will be PSW_" + wellIdentifierNameWidget->text() + "_N");};
   connect(wellIdentifierNameWidget, &QLineEdit::editingFinished, updateName);
   updateName();
   wellIdentifierNameWidget->setFixedWidth(150);
   return wellIdentifierNameWidget;
}

QWidget* ImportWellPopupVSET::createSkipWidget() const
{
   QWidget* skipWidget = new QWidget();
   QHBoxLayout* skipLayout = new QHBoxLayout(skipWidget);
   skipLayout->addWidget(new QLabel(" x = ", skipWidget), Qt::AlignRight);
   QSpinBox* spin = new QSpinBox(skipWidget);
   spin->setRange(1, 1e3);
   spin->setValue(importOptions_.interval);
   skipLayout->addWidget(spin);
   connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotHandleSetInterval(int)));
   return skipWidget;
}

void ImportWellPopupVSET::slotHandleSetInterval(int interval)
{
   importOptions_.interval = interval;
}

QWidget* ImportWellPopupVSET::createDistanceWidget() const
{
   QWidget* distanceWidget = new QWidget();
   QHBoxLayout* distanceLayout = new QHBoxLayout(distanceWidget);
   distanceLayout->addWidget(new QLabel(" distance [m] = ", distanceWidget), Qt::AlignRight);
   QSpinBox* spin = new QSpinBox(distanceWidget);
   spin->setRange(1, 1e6);
   spin->setValue(importOptions_.distance);
   distanceLayout->addWidget(spin);
   connect(spin, SIGNAL(valueChanged(int)), this, SLOT(slotHandleSetDistance(int)));
   return distanceWidget;
}

void ImportWellPopupVSET::slotHandleSetDistance(int distance)
{
   importOptions_.distance = distance;
}

QWidget* ImportWellPopupVSET::createWellsFileWidget()
{
   QWidget* wellsFileWidget = new QWidget();
   QHBoxLayout* wellsFileLayout = new QHBoxLayout(wellsFileWidget);
   QPushButton* button = new QPushButton("Select file", wellsFileWidget);
   wellsFileLayout->addWidget(button);
   selectedFileLabel_ = new QLabel(" selected file ", wellsFileWidget);
   wellsFileLayout->addWidget(selectedFileLabel_);
   connect(button, SIGNAL(clicked()), this, SLOT(slotSelectWellsFile()));
   return wellsFileWidget;
}

void ImportWellPopupVSET::slotSelectWellsFile()
{
   const QString fileName = QFileDialog::getOpenFileName(this,
                                                         "Select well location file",
                                                         "",
                                                         "Well-location files (*.*)");

   if (fileName.isEmpty())
   {
      return;
   }
   std::fstream fileStream(fileName.toStdString());
   if (!fileStream.good())
   {
      reject();
      messageBox::showErrorMessageBox("The chosen file cannot be opened. Either the file does not exist, or the permissions are wrong");
      return;
   }

   selectedFileLabel_->setText(fileName);
   importOptions_.xyPairs.clear();
   std::string line;
   while (fileStream)
   {
      std::getline(fileStream, line);
      if (line.empty())
      {
         return;
      }
      std::istringstream ss(line);
      std::string token;
      QPair<double, double> xyPair;

      std::getline(ss, token, ',');
      xyPair.first = std::stod(token);

      std::getline(ss, token, ',');
      xyPair.second = std::stod(token);
      importOptions_.xyPairs.append(xyPair);
   }
}

void ImportWellPopupVSET::slotHandleFilterOptionChanged(int option)
{
   filterLayout_->setCurrentIndex(option);
}

ImportOptionsVSET ImportWellPopupVSET::getImportOptions()
{
   if (filterLayout_->currentIndex() != 1) importOptions_.interval = 1;
   if (filterLayout_->currentIndex() != 2) importOptions_.distance = 0.0;
   if (filterLayout_->currentIndex() != 3) importOptions_.xyPairs.clear();

   importOptions_.depthNotTWT = (static_cast<QComboBox*>(propertyMappingTable_->cellWidget(0,0))->currentText() == "Depth");

   return importOptions_;
}

} // namespace casaWizard

