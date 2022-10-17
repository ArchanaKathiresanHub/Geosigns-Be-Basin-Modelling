//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "view/SacInputTab.h"

namespace casaWizard
{

namespace sac
{

namespace thermal
{

class ThermalInputTab : public SacInputTab
{
public:
   explicit ThermalInputTab(QWidget* parent = nullptr);

   const QPushButton* pushButtonImportTargets() const;
   QLineEdit* lineEditMinTCHP() const;
   QLineEdit* lineEditMaxTCHP() const;
private:
   QPushButton* m_pushImportTargets;
   QLineEdit* m_lineMinTCHP;
   QLineEdit* m_lineMaxTCHP;
};

} //namespace thermal

} //namespace sac

} //namespace casaWizard
