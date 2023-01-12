//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QTest>

namespace guiUtilities
{

const static int timeDelay = 200; //milliseconds

void openExistingFileInDialog(const QString& fileName, const bool accept = true);
void openNewFileInDialog(const QString& fileName, const bool accept = true);
void setIntInInputDialog(const int value, const bool accept = true);

template<typename Type>
void pressButtonInDialog(const bool accept);

template<typename Type>
Type* getQChild(const QString& name, const QWidget* parent);

template <class P> QList<P> topLevelWidgets();
template <class P> P topLevelWidget();


} // namespace TestUtilities

#include "GuiTestUtilities.tpp" //template implementations
