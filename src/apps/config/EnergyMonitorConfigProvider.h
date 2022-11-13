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
			static const char emonConfigFile[];				// Configuration file name.
			static const char rotationsParamName[];			// Name of saved parameter.

		public:
			/*
				Injects WiFiManager parameters.
				@param manager WiFiManager reference.
			*/
			void injectManagerParameters(WiFiManager& manager) override;

			/*
				Captures value of injected WiFiManager parameters.
				@param manager WiFiManager reference.
			*/
			void captureManagerParameters(WiFiManager& manager) override;

			/*
				Reads configured rotationsPerKwh to the value passed by reference.
				@param rotationsPerKwh Reference to number of rotations per kwh.
				@return True on success read and conversion, otherwise false.
			*/
			bool setupRotations(unsigned short& rotationsPerKwh);
	};
}