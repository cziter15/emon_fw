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

namespace apps::config
{
	class EnergyMonitorConfig : public ksf::ksApplication
	{
		public:
			static const char emonDeviceName[];				// Static table of characters with device name.

			/*
				Initializes EnergyMonitorConfig application.

				@return True on success, false on fail.
			*/
			bool init() override;

			/*
				Handles EnergyMonitorConfig realtime application logic.

				@return True on success, false on fail.
			*/
			bool loop() override;
	};
}
