/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <ksIotFrameworkLib.h>

class WiFiManager;

namespace apps::config
{
	class EnergyMonitorConfigProvider : public ksf::comps::ksConfigProvider
	{
		KSF_RTTI_DECLARATIONS(EnergyMonitorConfigProvider, ksf::comps::ksConfigProvider)

		protected:
			static const char emonConfigFile[];
			static const char rotationsParamName[];

		public:
			void injectManagerParameters(WiFiManager& manager) override;
			void captureManagerParameters(WiFiManager& manager) override;
			bool setupRotations(unsigned short& rotationsPerKwh);
	};
}