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
		params.push_back(new WiFiManagerParameter(rotationsParamName, rotationsParamName, config_file.getParam(rotationsParamName, "").c_str(), 5));
		manager.addParameter(params.back());
	}
}

void EnergyMonitorConfigProvider::captureManagerParameters(WiFiManager& manager)
{
	USING_CONFIG_FILE(emonConfigFile)
	{
		for (auto& param : params)
			config_file.setParam(param->getID(), param->getValue());

		for (auto& param : params)
			delete param;

		params.clear();
	}
}