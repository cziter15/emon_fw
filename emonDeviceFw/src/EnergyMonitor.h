#pragma once

#include <ksIotFrameworkLib.h>

#define EMON_SENSOR_PROBES 400
#define EMON_TIMER_INTERVAL 50
#define MAX_ANA_VALUE 1024
#define EMON_SEC_TIMER 1000
#define EMON_SENSOR_TIMEOUT 600000UL
#define EMON_STABILIZATION_PROBE_COUNT 10
#define EMON_DEVIATION_UPHILL 0.2f
#define EMON_DEVIATION_STABILIZATION 0.1f

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
		DOMINANT
	};
}

namespace curve_state
{
	enum TYPE
	{
		WAIT_PREWARM = 0,
		WAIT_UPHILL,
		WAIT_STABILIZE
	};
}


class EnergyMonitor : public ksf::ksApplication
{
	protected:
		debug_mode_type::TYPE debug_mode = debug_mode_type::NONE;
		curve_state::TYPE current_curve_state = curve_state::WAIT_PREWARM;

		std::weak_ptr<ksf::ksMqttConnector> mqtt_wp;
		std::weak_ptr<ksf::ksLed> statusLed_wp, eventLed_wp;

		std::shared_ptr<ksf::ksEventHandle> connEventHandle_sp, msgEventHandle_sp, disEventHandle_sp;
		std::shared_ptr<ksf::ksEventHandle> sensorUpdateEventHandle_sp, secTimerEventHandle_sp;

		double initialKwh = -1;
		double curWatts = -1.0;
		double accumulativeWatts = 0.0;

		unsigned short rotationsPerKwh = 0;
		unsigned long prevPulseAtMillis = 0;
		unsigned long pulseCount = 0;
		
		unsigned short secondsCounter = 0;
		unsigned short stabilizationCounter = 0;

		std::vector<unsigned short> buffered_values;
		std::vector<unsigned short> dominant_buffer;

		unsigned short last_val_idx = 0;

		bool wantUpper = true;

		void onMqttMessage(const String& topic, const String& payload);
		void onMqttConnected();
		void onMqttDisconnected();

		void blackLineDetected();

		void onBlackLineSensorTimer();
		void onAvgCalculationTimer();

		void updateWatts(double currentWatts);

		unsigned short getDominantAsInt(std::vector<unsigned short>& valAttr);

	public:
		bool init() override;
		bool loop() override;
};