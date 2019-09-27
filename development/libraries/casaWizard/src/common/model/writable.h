// Interface for class that can read and write
#pragma once

namespace casaWizard
{

class ScenarioReader;
class ScenarioWriter;

class Writable
{
public:
  virtual void writeToFile(ScenarioWriter& writer) const = 0;
  virtual void readFromFile(const ScenarioReader& reader) = 0;
  virtual void clear() = 0;
};

}
