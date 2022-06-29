//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

//Struct of proxy fitting settings. Meaning as given in the CompoundProxyCollection::calculate function header:
//[in] order            maximum order of monomials to include in the proxy
//[in] modelSearch      flag indicating whether to search for optimal proxies
//[in] targetR2         adjusted R^2 value above which we accept the model
//[in] confLevel        needed for significance test of model increments
struct TestInfo
{
   TestInfo():
      order(0),
      modelSearch(true),
      targetR2(1.0),
      confLevel(0.5)
   {}

   unsigned int order;
   bool modelSearch;
   double targetR2;
   double confLevel;
};
