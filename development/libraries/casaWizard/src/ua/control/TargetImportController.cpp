

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TargetImportController.h"

#include "model/PredictionTargetCreator.h"
#include "model/TargetInputInfo.h"

#include <QStringList>
#include <QMessageBox>
#include <QPushButton>

namespace casaWizard
{

namespace ua
{

TargetImportController::TargetImportController(PredictionTargetManager& predictionTargetManager,
                                               QObject *parent):
   QObject(parent),
   m_predictionTargetManager(predictionTargetManager)
{

}

PredictionTargetManager& TargetImportController::getPredictionTargetManager() const
{
   return m_predictionTargetManager;
}

void TargetImportController::slotImportAccepted()
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

void TargetImportController::slotClearAndWritePredictionTargets()
{
   m_predictionTargetManager.clear();
   slotImportPredictionTargets();
}

} // namespace ua

} // namespace casaWizard



