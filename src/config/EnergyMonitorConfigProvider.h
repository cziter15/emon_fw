#pragma once
#include <ksIotFrameworkLib.h>

class WiFiManager;

class EnergyMonitorConfigProvider : public ksf::comps::ksConfigProvider
{
	protected:
		static const char emonConfigFile[];
		static const char rotationsParamName[];

	public:
		void injectManagerParameters(WiFiManager& manager) override;
		void captureManagerParameters(WiFiManager& manager) override;
		bool setupRotations(unsigned short& rotationsPerKwh);
};