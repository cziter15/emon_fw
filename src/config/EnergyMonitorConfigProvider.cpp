#include "EnergyMonitorConfigProvider.h"

const char EnergyMonitorConfigProvider::emonConfigFile[] = "emon.conf";
const char EnergyMonitorConfigProvider::rotationsParamName[] = "rotationsPerKwh";

bool EnergyMonitorConfigProvider::setupRotations(unsigned short& outRotationsPerKwh)
{
	USING_CONFIG_FILE(emonConfigFile)
	{
		outRotationsPerKwh = (unsigned short)(config_file.getParam(rotationsParamName).toInt());
	}

	return outRotationsPerKwh > 0;
}

void EnergyMonitorConfigProvider::injectManagerParameters(WiFiManager& manager)
{
	USING_CONFIG_FILE(emonConfigFile)
	{
		addNewParam(manager, rotationsParamName, config_file.getParam(rotationsParamName).c_str(), 5);
	}
}

void EnergyMonitorConfigProvider::captureManagerParameters(WiFiManager& manager)
{
	USING_CONFIG_FILE(emonConfigFile)
	{
		for (auto& param : params)
			config_file.setParam(param->getID(), param->getValue());

		params.clear();
	}
}