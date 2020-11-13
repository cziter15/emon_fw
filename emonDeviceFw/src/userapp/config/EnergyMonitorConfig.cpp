#include "EnergyMonitorConfig.h"
#include "EnergyMonitorConfigProvider.h"

#include "../board.h"

const char EnergyMonitorConfig::emonDeviceName[] = "EnergyMonitor";

bool EnergyMonitorConfig::init()
{
	addComponent<ksf::ksLed>(STATUS_LED_PIN);
	addComponent<ksf::ksLed>(EVENT_LED_PIN);

	addComponent<ksf::ksWiFiConfigurator>(emonDeviceName);
	addComponent<ksf::ksMqttConfigProvider>();
	addComponent<EnergyMonitorConfigProvider>();

	return ksApplication::init();
}

bool EnergyMonitorConfig::loop()
{
	return ksApplication::loop();
}