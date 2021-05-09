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

	mqtt_wp = addComponent<ksf::ksMqttConnector>();
	statusLed_wp = addComponent<ksf::ksLed>(STATUS_LED_PIN);
	eventLed_wp = addComponent<ksf::ksLed>(EVENT_LED_PIN);

	/* Setup reset button */
	addComponent<ksf::ksResetButton>(CFG_PUSH_PIN, LOW);

	auto sensor_timer_sp = addComponent<ksf::ksTimer>(EMON_TIMER_INTERVAL, true).lock();
	auto sec_timer_sp = addComponent<ksf::ksTimer>(EMON_SEC_TIMER, true).lock();
	
	if (!ksApplication::init())
		return false;
	
	ArduinoOTA.begin();
	ArduinoOTA.setPassword("ota_ksiotframework");

	EnergyMonitorConfigProvider configProvider;
	configProvider.init(this);

	if (!configProvider.setupRotations(rotationsPerKwh))
		return false;
	
	if (auto mqtt_sp = mqtt_wp.lock())
	{
		mqtt_sp->onMesssage->registerEvent(msgEventHandle_sp, std::bind(&EnergyMonitor::onMqttMessage, this, _1, _2));
		mqtt_sp->onConnected->registerEvent(connEventHandle_sp, std::bind(&EnergyMonitor::onMqttConnected, this));
		mqtt_sp->onDisconnected->registerEvent(disEventHandle_sp, std::bind(&EnergyMonitor::onMqttDisconnected, this));
	}

	if (sensor_timer_sp)
		sensor_timer_sp->onTimerExpired->registerEvent(sensorUpdateEventHandle_sp, std::bind(&EnergyMonitor::onBlackLineSensorTimer, this));
	
	if (sec_timer_sp)
		sec_timer_sp->onTimerExpired->registerEvent(secTimerEventHandle_sp, std::bind(&EnergyMonitor::onAvgCalculationTimer, this));

	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);

	pinMode(ANA_PIN, INPUT);

	return true;
}

void EnergyMonitor::onMqttDisconnected()
{
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(500);
}

void EnergyMonitor::onMqttConnected()
{
	if (auto statusLed_sp = statusLed_wp.lock())
		statusLed_sp->setBlinking(0);

	if (auto mqtt_sp = mqtt_wp.lock())
	{
		mqtt_sp->subscribe("totalkWh");
		mqtt_sp->subscribe("clearkwh");
		mqtt_sp->subscribe("dbg");
	}
}

void EnergyMonitor::onMqttMessage(const String& topic, const String& payload)
{
	if (auto mqtt_sp = mqtt_wp.lock())
	{
		if (topic.equals("totalkWh") && initialKwh < 0)
		{
			initialKwh = payload.toDouble();
			mqtt_sp->unsubscribe("totalkWh");
		}
		else if (topic.equals("clearkwh"))
		{
			pulseCount = 0;
			initialKwh = 0;

			mqtt_sp->publish("totalkWh", String('0'), true);

			time_t rawtime; time(&rawtime);
			if (struct tm* timeinfo = localtime(&rawtime))
			{
				String date =
					String(timeinfo->tm_mday) + "." + String(timeinfo->tm_mon + 1) + "." + String(timeinfo->tm_year + 1900) + " " +
					String(timeinfo->tm_hour) + ":" + String(timeinfo->tm_min) + ":" + String(timeinfo->tm_sec) + " UTC";

				mqtt_sp->publish("kWhResetTime", date);
			}
		}
		else if (topic.equals("dbg"))
		{
			debug_mode = (debug_mode_type::TYPE)payload.toInt();
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
			if (auto mqtt_sp = mqtt_wp.lock())
			{
				double avg5minWatts = accumulativeWatts / secondsCounter;
				mqtt_sp->publish("5minAvgWatts", String(avg5minWatts, 0));

				double totalkWh = (double)pulseCount / (double)rotationsPerKwh + initialKwh;
				mqtt_sp->publish("totalkWh", String(totalkWh), true);
			}

			secondsCounter = 0;
			accumulativeWatts = 0;
		}
	}
}

void EnergyMonitor::updateWatts(double currentWatts)
{
	curWatts = currentWatts;

	/* If no debug, send calculated watts */
	if (debug_mode == debug_mode_type::NONE)
		if (auto mqtt_sp = mqtt_wp.lock())
			mqtt_sp->publish("watts", String(curWatts, 0));

	prevPulseAtMillis = millis();
}

void EnergyMonitor::blackLineDetected()
{
	++pulseCount;
	double pulseTime = millis() - prevPulseAtMillis;
	updateWatts((unsigned long)(3600000.0 / ((double)rotationsPerKwh * pulseTime) * 1000.0));

	prevPulseAtMillis = millis();

	if (auto eventLed_sp = eventLed_wp.lock())
		eventLed_sp->setBlinking(75, 6);
}

void EnergyMonitor::onBlackLineSensorTimer()
{
	float lastRecordsAverage = 0;
	float analog_value = analogRead(ANA_PIN);

	buffered_values[++last_val_idx % EMON_SENSOR_PROBES] = (unsigned short)analog_value;

	for (unsigned int idx = 0; idx < EMON_SENSOR_PROBES; ++idx)
		lastRecordsAverage += buffered_values[(idx + last_val_idx) % EMON_SENSOR_PROBES];

	lastRecordsAverage /= EMON_SENSOR_PROBES;

	float currentDeviation = analog_value / lastRecordsAverage;

	if (auto mqtt_sp = mqtt_wp.lock())
	{
		switch (debug_mode)
		{
			case debug_mode_type::DEVIATION:	mqtt_sp->publish("watts", String(currentDeviation, 2));		break;
			case debug_mode_type::RAW_VALUE:	mqtt_sp->publish("watts", String(analog_value, 2));			break;
			case debug_mode_type::AVG_VALUES:	mqtt_sp->publish("watts", String(lastRecordsAverage, 2));	break;
			default: break;
		}
	}

	switch (current_curve_state)
	{
		case curve_state::WAIT_PREWARM:
			if (last_val_idx > EMON_SENSOR_PROBES)
				current_curve_state = curve_state::WAIT_STABILIZE;
		break;

		case curve_state::WAIT_STABILIZE:
			if (fabsf(currentDeviation - 1.0f) <= 0.05f)
				current_curve_state = curve_state::WAIT_UP;
		break;

		case curve_state::WAIT_UP:
			if (currentDeviation > EMON_UP_TRESHOLD)
			{
				current_curve_state = curve_state::WAIT_FALL;
				blackLineDetected();
			}
		break;

		case curve_state::WAIT_FALL:
			if (currentDeviation < EMON_DOWN_TRESHOLD)
				current_curve_state = curve_state::WAIT_STABILIZE;
		break;

	}

	if (millis() - prevPulseAtMillis > EMON_SENSOR_TIMEOUT)
		updateWatts(0);
}

bool EnergyMonitor::loop()
{
	ArduinoOTA.handle();
	return ksApplication::loop();
}