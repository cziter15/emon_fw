/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "EnergyMonitorConfigProvider.h"

namespace apps::config
{
	#define EMON_CONF_FILENAME_PGM 			PGM_("emon.conf")
	#define ROTATIONS_PARAM_TEXT_PGM 		PGM_("rotationsPerKwh")

	bool EnergyMonitorConfigProvider::setupRotations(unsigned short& outRotationsPerKwh)
	{
		outRotationsPerKwh = 0;

		USING_CONFIG_FILE(EMON_CONF_FILENAME_PGM)
		{
			auto& rotationsParamContent{config_file.getParam(ROTATIONS_PARAM_TEXT_PGM)};
			ksf::from_chars(rotationsParamContent, outRotationsPerKwh);
		}

		return outRotationsPerKwh > 0;
	}

	void EnergyMonitorConfigProvider::injectManagerParameters(WiFiManager& manager)
	{
		USING_CONFIG_FILE(EMON_CONF_FILENAME_PGM)
		{
			addNewParamWithConfigDefault(manager, config_file, ROTATIONS_PARAM_TEXT_PGM, 5);
		}
	}

	void EnergyMonitorConfigProvider::captureManagerParameters(WiFiManager& manager)
	{
		USING_CONFIG_FILE(EMON_CONF_FILENAME_PGM)
		{
			for (auto& param : params)
				config_file.setParam(param.second->getID(), param.second->getValue());

			params.clear();
		}
	}
}