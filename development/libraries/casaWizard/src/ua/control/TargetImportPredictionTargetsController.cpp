

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportPredictionTargetsController.h"

#include "model/calibrationTargetManager.h"
#include "model/predictionTargetManager.h"
#include "model/PredictionTargetCreator.h"
#include "model/TargetInputInfoFromASCII.h"

#include "view/TargetImportDialogAscii.h"

#include <QStringList>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>

namespace casaWizard
{
namespace ua
{
TargetImportPredictionTargetsController::TargetImportPredictionTargetsController(const CalibrationTargetManager& calibrationTargetManager,
                                                                                 PredictionTargetManager& predictionTargetManager,
                                                                                 QObject *parent):
   QObject(parent),
   m_calibrationTargetManager(calibrationTargetManager),
   m_predictionTargetManager(predictionTargetManager),
   m_targetImportDialogAscii(new TargetImportDialogAscii())
{
   if (!slotPushSelectFileClicked())
   {
      return;
   }

   const QStringList surfaceNames = m_predictionTargetManager.validSurfaceNames();
   m_targetImportDialogAscii->updateSurfaceTable(surfaceNames);

   connect(m_targetImportDialogAscii, SIGNAL(accepted()), this, SLOT(slotImportAccepted()));
   connect(m_targetImportDialogAscii->pushSelectFile(), SIGNAL(clicked()), this, SLOT(slotPushSelectFileClicked()));

   m_targetImportDialogAscii->exec();
}

TargetImportPredictionTargetsController::~TargetImportPredictionTargetsController()
{
   delete m_targetImportDialogAscii;
}

void TargetImportPredictionTargetsController::slotImportAccepted()
{
   if (m_predictionTargetManager.amountAtAge0() > 0)
   {
      QMessageBox overwriteData(QMessageBox::Icon::Information,
                                "The target table already has targets.",
                                "Would you like to overwrite or append the new targets?");
      QPushButton* appendButton = overwriteData.addButton("Append", QMessageBox::RejectRole);
      QPushButton* overwriteButton = overwriteData.addButton("Overwrite", QMessageBox::AcceptRole);
      connect(appendButton, SIGNAL(clicked()), this, SLOT(slotImportPredictionTargets()));
      connect(overwriteButton, SIGNAL(clicked()), this, SLOT(slotClearAndWritePredictionTargets()));
      overwriteData.exec();
      //If the dialog is closed without selecting append or overwrite, no prediction targets are imported.
   }
   else
   {
      slotImportPredictionTargets();
   }
}

void TargetImportPredictionTargetsController::slotImportPredictionTargets()
{
   //get data from Dialog and make prediction targets
   TargetInputInfoFromASCII info(m_targetImportDialogAscii->lineEditFile()->text(),
                                 m_targetImportDialogAscii->surfaceSelectionStates(),
                                 m_targetImportDialogAscii->temperatureTargetsSelected(),
                                 m_targetImportDialogAscii->vreTargetsSelected(),
                                 m_targetImportDialogAscii->depthInput(),
                                 m_targetImportDialogAscii->lineEditName()->text());

   PredictionTargetCreator creator(info, m_predictionTargetManager);
   creator.createTargets();
}

void TargetImportPredictionTargetsController::slotClearAndWritePredictionTargets()
{
   m_predictionTargetManager.clear();
   slotImportPredictionTargets();
}

bool TargetImportPredictionTargetsController::slotPushSelectFileClicked()
{
   QString filename = QFileDialog::getOpenFileName(nullptr,
                                                   "Select Prediction targets",
                                                   m_targetImportDialogAscii->lineEditFile()->text(),
                                                   "Text files (*.txt)");

   if (filename == "")
   {
      return false;
   }
   m_targetImportDialogAscii->lineEditFile()->setText(filename);
   return true;
}

} // namespace ua

} // namespace casaWizard



