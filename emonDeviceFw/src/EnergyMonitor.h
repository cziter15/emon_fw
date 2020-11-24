#pragma once

#include <ksIotFrameworkLib.h>

#define EMON_SENSOR_PROBES 20
#define EMON_UP_TRESHOLD 1.15f
#define EMON_DOWN_TRESHOLD 0.85f

#define EMON_TIMER_INTERVAL 50
#define EMON_SEC_TIMER 1000
#define EMON_SENSOR_TIMEOUT 600000UL

namespace ksf
{
	class ksMqttConnector;
	class ksEventHandle;
	class ksLed;
}

namespace debug_mode_type
{
	enum TYPE
	{
		NONE = 0,
		DEVIATION,
		RAW_VALUE,
		AVG_VALUES
	};
}

namespace curve_state
{
	enum TYPE
	{
		WAIT_PREWARM = 0,
		WAIT_UP,
		WAIT_FALL,
		WAIT_STABILIZE
	};
}


class EnergyMonitor : public ksf::ksApplication
{
	protected:
		debug_mode_type::TYPE debug_mode = debug_mode_type::NONE;
		curve_state::TYPE current_curve_state = curve_state::WAIT_PREWARM;

		std::shared_ptr<ksf::ksMqttConnector> mqtt;

		std::shared_ptr<ksf::ksLed> statusLed, eventLed;
		std::shared_ptr<ksf::ksEventHandle> connEventHandle, msgEventHandle, disEventHandle;
		std::shared_ptr<ksf::ksEventHandle> sensorUpdateEventHandle, secTimerEventHandle;

		double initialKwh = -1;
		double curWatts = -1.0;
		double accumulativeWatts = 0.0;

		unsigned short rotationsPerKwh = 0;
		unsigned long prevPulseAtMillis = 0;
		unsigned long pulseCount = 0;
		
		unsigned short secondsCounter = 0;
		
		unsigned short buffered_values[EMON_SENSOR_PROBES];
		unsigned short last_val_idx = 0;

		bool wantUpper = true;

		void onMqttMessage(const String& topic, const String& payload);
		void onMqttConnected();
		void onMqttDisconnected();

		void blackLineDetected();

		void onBlackLineSensorTimer();
		void onAvgCalculationTimer();

		void updateWatts(double currentWatts);

	public:
		bool init() override;
		bool loop() override;
};