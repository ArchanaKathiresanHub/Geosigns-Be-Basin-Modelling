#include "optimizedLithofractionScript.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

OptimizedLithofractionScript::OptimizedLithofractionScript(const QString& baseDirectory, QString scriptFileName) :
   OptimalValueExtractionScript(baseDirectory, scriptFileName)
{}

void OptimizedLithofractionScript::writeScript(QFile* file)
{
   QTextStream out(file);
   out << "loadstate \"casa_state.bin\" \"bin\"\n";
   out << "exportDataTxt \"DoEParameters\" \"optimalLithofractions.txt\" \"BestMatchedCase\"\n";
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard
