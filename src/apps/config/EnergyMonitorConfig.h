/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
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
			static const char emonDeviceName[];

			bool init() override;
			bool loop() override;
	};
}
