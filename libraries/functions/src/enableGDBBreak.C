#include "enableGDBBreak.h"

namespace functions { namespace enableGDBBreak {

void enableBreak1()
{
  static int n = 0;
  ++n;
}

void enableBreak2()
{
  static int n = 0;
  ++n;
}

} } // namespace functions::enableGDBBreak
