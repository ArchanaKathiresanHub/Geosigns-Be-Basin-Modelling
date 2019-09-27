// Function to convert the data read from the track1d output file and store it in the SAC scenario

namespace casaWizard
{

namespace sac
{

class Case3DTrajectoryReader;
class SACScenario;

namespace case3DTrajectoryConvertor
{

void convertToScenario(const Case3DTrajectoryReader& reader, SACScenario& scenario, const bool optimized = true);

}

} // namespace sac

} // namespace casaWizard
