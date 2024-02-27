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
	const char EMON_CONF_FILENAME_PGM[] 	PROGMEM {"emon.conf"};
	const char ROTATIONS_PARAM_TEXT_PGM[] 	PROGMEM {"rotationsPerKwh"};

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

	void EnergyMonitorConfigProvider::readParams()
	{
		USING_CONFIG_FILE(EMON_CONF_FILENAME_PGM)
		{
			addNewParamWithConfigDefault(config_file, ROTATIONS_PARAM_TEXT_PGM, 5);
		}
	}

	void EnergyMonitorConfigProvider::saveParams()
	{
		USING_CONFIG_FILE(EMON_CONF_FILENAME_PGM)
		{
			for (auto& param : params)
				config_file.setParam(param.id.c_str(), param.value);

			params.clear();
		}
	}
}