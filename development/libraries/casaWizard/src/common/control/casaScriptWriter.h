// Namespace with method to write casa scripts to file

class QString;

namespace casaWizard
{

class CasaScript;

namespace casaScriptWriter
{

  bool writeCasaScript(CasaScript& script);
  bool writeCasaScriptFilterOutDataDir(CasaScript& script, const QString& filteredDataDir);

}

} // namespace casaWizard
