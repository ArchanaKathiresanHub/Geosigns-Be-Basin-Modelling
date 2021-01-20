//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "customtab.h"

namespace casaWizard
{

CustomTab::CustomTab(QWidget *parent) : QTabWidget(parent)
{
  setStyleSheet("QTabWidget::pane { background : rgb(255,255,255)}"
                "QTabBar::tab { "
                    "background : rgb(239,239,239);"
                    "border: 2px solid rgb(239,239,239);"
                    "border-bottom-color: #484848;"
                    "font-size : 15px;"
                    "color: #484848;"
                    "min-width: 8ex;"
                    "min-height: 24px;"
                "}"
                "QTabBar::tab:selected {"
                    "border-bottom : 4px solid #0C7CC7;"
                    "color: #0C7CC7;"
                    "font-weight: bold;"
                    "margin-right: 2px;"
                    "margin-left: 2px;"


                "}"
                "QTabBar::tab:!selected {"
                    "margin-top: 0px;"

                "}"
                );
}

} // namespace casaWizard
