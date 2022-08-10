

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportPredictionTargetsController.h"

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
TargetImportPredictionTargetsController::TargetImportPredictionTargetsController(PredictionTargetManager& predictionTargetManager,
                                                                                 QObject *parent):
   TargetImportController(predictionTargetManager,
                          parent),
   m_targetImportDialogAscii(new TargetImportDialogAscii())
{
   if (!slotPushSelectFileClicked())
   {
      return;
   }

   const QStringList surfaceNames = getPredictionTargetManager().validSurfaceNames();
   m_targetImportDialogAscii->updateSurfaceTable(surfaceNames);

   connect(m_targetImportDialogAscii, SIGNAL(accepted()), this, SLOT(slotImportAccepted()));
   connect(m_targetImportDialogAscii->pushSelectFile(), SIGNAL(clicked()), this, SLOT(slotPushSelectFileClicked()));

   m_targetImportDialogAscii->exec();
}

TargetImportPredictionTargetsController::~TargetImportPredictionTargetsController()
{
   delete m_targetImportDialogAscii;
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

   PredictionTargetCreator creator(info, this->getPredictionTargetManager());
   creator.createTargets();
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



