#pragma once
#include <ksIotFrameworkLib.h>

class EnergyMonitorConfig : public ksf::ksApplication
{
	public:
		static const char emonDeviceName[];

		bool init() override;
		bool loop() override;
};