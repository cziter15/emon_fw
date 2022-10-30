/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "EnergyMonitorConfigProvider.h"

namespace apps::config
{
	const char EnergyMonitorConfigProvider::emonConfigFile[] = "emon.conf";
	const char EnergyMonitorConfigProvider::rotationsParamName[] = "rotationsPerKwh";

	bool EnergyMonitorConfigProvider::setupRotations(unsigned short& outRotationsPerKwh)
	{
		USING_CONFIG_FILE(emonConfigFile)
		{
			auto& rotationsParamContent{config_file.getParam(rotationsParamName)};
			ksf::from_chars(rotationsParamContent, outRotationsPerKwh);
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
}
