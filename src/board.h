/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

/* Pin number used to handle reset button functionality. */
#define CFG_PUSH_PIN 0
/* Pin number for reading analog value (there is only one on ESP8266). */
#define ANA_PIN A0

/* Pin number of the LED used as status (MQTT connection) idicator. */
#define STATUS_LED_PIN 4
/* Pin number of the LED used as event (measurement) idicator. */
#define EVENT_LED_PIN 5