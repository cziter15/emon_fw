#include "src/EnergyMonitor.h"
#include "src/config/EnergyMonitorConfig.h"

#ifdef ESP32
#include "SPIFFS.h"
#else
#include "FS.h"
#endif

#include "user_interface.h"

// the setup function runs once when you press reset or power the board
void setup()
{
	uint32_t chipId = ESP.getChipId();
	auto chipIdBytes = (unsigned char*)&chipId;

	uint8_t mac[6]{ 0xfa, 0xf1, chipIdBytes[0], chipIdBytes[1], chipIdBytes[2], chipIdBytes[3]};
	wifi_set_macaddr(STATION_IF, mac);

	SPIFFS.begin();
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	RUN_APP_BLOCKING_LOOPED(EnergyMonitor)
	RUN_APP_BLOCKING_LOOPED(EnergyMonitorConfig)
}