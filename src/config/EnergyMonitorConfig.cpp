#include "EnergyMonitorConfig.h"
#include "EnergyMonitorConfigProvider.h"

#include "board.h"

const char EnergyMonitorConfig::emonDeviceName[] = "EnergyMonitor";

bool EnergyMonitorConfig::init()
{
	addComponent<ksf::comps::ksLed>(STATUS_LED_PIN);
	addComponent<ksf::comps::ksLed>(EVENT_LED_PIN);

	addComponent<ksf::comps::ksWiFiConfigurator>(emonDeviceName);
	addComponent<ksf::comps::ksMqttConfigProvider>();
	addComponent<EnergyMonitorConfigProvider>();

	return ksApplication::init();
}

bool EnergyMonitorConfig::loop()
{
	return ksApplication::loop();
}