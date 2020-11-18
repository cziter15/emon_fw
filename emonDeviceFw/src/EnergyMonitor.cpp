#include "EnergyMonitor.h"
#include "config/EnergyMonitorConfig.h"
#include "config/EnergyMonitorConfigProvider.h"
#include "board.h"

#include <ksIotFrameworkLib.h>

#include "ArduinoOTA.h"

using namespace std::placeholders;

bool EnergyMonitor::init()
{
	addComponent<ksf::ksWifiConnector>(EnergyMonitorConfig::emonDeviceName);
	addComponent<ksf::ksMqttDebugResponder>();
	mqtt = addComponent<ksf::ksMqttConnector>();
	statusLed = addComponent<ksf::ksLed>(STATUS_LED_PIN);
	eventLed = addComponent<ksf::ksLed>(EVENT_LED_PIN);

	auto sensor_timer = addComponent<ksf::ksTimer>(EMON_TIMER_INTERVAL, true);
	auto sec_timer = addComponent<ksf::ksTimer>(EMON_SEC_TIMER, true);

	ArduinoOTA.begin();
	ArduinoOTA.setPassword("ota_ksiotframework");

	ksApplication::init();

	EnergyMonitorConfigProvider configProvider;
	configProvider.init(this);

	if (!configProvider.setupRotations(rotationsPerKwh))
		return false;

	mqtt->onMesssage.registerEvent(msgEventHandle, std::bind(&EnergyMonitor::onMqttMessage, this, _1, _2));
	mqtt->onConnected.registerEvent(connEventHandle, std::bind(&EnergyMonitor::onMqttConnected, this));
	mqtt->onDisconnected.registerEvent(disEventHandle, std::bind(&EnergyMonitor::onMqttDisconnected, this));

	sensor_timer->onTimerExpired.registerEvent(sensorUpdateEventHandle, std::bind(&EnergyMonitor::onBlackLineSensorTimer, this));
	sec_timer->onTimerExpired.registerEvent(secTimerEventHandle, std::bind(&EnergyMonitor::onAvgCalculationTimer, this));

	statusLed->setBlinking(500);

	pinMode(A0, INPUT);

	/* Set all sensor probes to USHRT_MAX to prevent fake detections */
	for (unsigned int idx = 0; idx < EMON_SENSOR_PROBES; ++idx)
		buffered_values[idx] = USHRT_MAX;

	return true;
}

void EnergyMonitor::onMqttDisconnected()
{
	statusLed->setBlinking(500);
}

void EnergyMonitor::onMqttConnected()
{
	statusLed->setBlinking(0);

	mqtt->subscribe("totalkWh");
	mqtt->subscribe("clearkwh");
}

void EnergyMonitor::onMqttMessage(const String& topic, const String& payload)
{
	if (topic.equals("totalkWh") && initialKwh < 0)
	{
		initialKwh = payload.toDouble();
		mqtt->unsubscribe("totalkWh");
	}
	else if (topic.equals("clearkwh"))
	{
		pulseCount = 0;
		initialKwh = 0;

		mqtt->publish("totalkWh", String('0'), true);

		time_t rawtime; time(&rawtime);
		if (struct tm* timeinfo = localtime(&rawtime))
		{
			String date =
				String(timeinfo->tm_mday) + "." + String(timeinfo->tm_mon + 1) + "." + String(timeinfo->tm_year + 1900) + " " +
				String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min) + ":" + String(timeinfo->tm_sec) + " UTC";

			mqtt->publish("kWhResetTime", date);
		}
	}
}

void EnergyMonitor::onAvgCalculationTimer()
{
	if (curWatts >= 0)
	{
		++secondsCounter;
		accumulativeWatts += curWatts;

		if (secondsCounter >= 300)
		{
			double avg5minWatts = accumulativeWatts / secondsCounter;
			mqtt->publish("5minAvgWatts", String(avg5minWatts, 0));

			double totalkWh = pulseCount / (double)rotationsPerKwh + initialKwh;
			mqtt->publish("totalkWh", String(totalkWh), true);

			secondsCounter = 0;
			accumulativeWatts = 0;
		}
	}
}

void EnergyMonitor::updateWatts(double currentWatts)
{
	curWatts = currentWatts;
	mqtt->publish("watts", String(curWatts, 0));
	prevPulseAtMillis = millis();
}

void EnergyMonitor::blackLineDetected()
{
	++pulseCount;
	double pulseTime = millis() - prevPulseAtMillis;
	updateWatts((unsigned long)(3600000.0 / ((double)rotationsPerKwh * pulseTime) * 1000.0));

	prevPulseAtMillis = millis();
	eventLed->setBlinking(75, 6);
}

void EnergyMonitor::onBlackLineSensorTimer()
{
	float lastRecordsAverage = 0;
	float analog_value = analogRead(A0);

	buffered_values[++last_val_idx % EMON_SENSOR_PROBES] = (unsigned short)analog_value;

	for (unsigned int idx = 0; idx < EMON_SENSOR_PROBES; ++idx)
		lastRecordsAverage += buffered_values[(idx + last_val_idx) % EMON_SENSOR_PROBES];

	lastRecordsAverage /= EMON_SENSOR_PROBES;

	float currentDeviation = analog_value / lastRecordsAverage;

	if (wantUpper && (currentDeviation > EMON_SENSOR_UP_TRESHOLD))
		wantUpper = false;

	if (!wantUpper && (currentDeviation < EMON_SENSOR_DOWN_TRESHOLD))
	{
		blackLineDetected();
		wantUpper = true;
	}

	if (millis() - prevPulseAtMillis > EMON_SENSOR_TIMEOUT)
		updateWatts(0);
}

bool EnergyMonitor::loop()
{
	ArduinoOTA.handle();
	return ksApplication::loop();
}