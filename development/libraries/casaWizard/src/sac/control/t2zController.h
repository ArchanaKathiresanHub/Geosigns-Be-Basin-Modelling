#pragma once

#include <QObject>
#include <QVector>

class QString;
class QTableWidgetItem;

namespace casaWizard
{

class ScriptRunController;

namespace sac
{

class SACScenario;
class T2Ztab;

class T2Zcontroller : public QObject
{
  Q_OBJECT

public:
  T2Zcontroller(T2Ztab* sacTab,
                SACScenario& casaScenario,
                ScriptRunController& scriptRunController,
                QObject* parent);

private slots:
  void slotPushButtonSACrunT2ZClicked();
  void slotSpinBoxReferenceSurfaceValueChanged(int referenceSurface);
  void slotSpinBoxLastSurfaceValueChanged(int lastSurface);

private:
  T2Ztab* t2zTab_;
  SACScenario& casaScenario_;
  ScriptRunController& scriptRunController_;
};

} // namespace sac

} // namespace casaWizard
