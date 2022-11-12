/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "LineSensor.h"
#include <cmath>
#include <Arduino.h>

namespace apps::emon::components::utils
{
	LineSensor::LineSensor(uint16_t probeCount, uint16_t maxValue, uint8_t pin)
		: pin(pin)
	{
		if (pin != std::numeric_limits<uint8_t>::max())
		{
			readingHistory.resize(probeCount);
			occurencesTable.resize(maxValue);
			pinMode(pin, INPUT);
		}
	}

	void LineSensor::pushReading(uint16_t value)
	{
		/* Head value pointer assignment. */
		auto headValPtr = &readingHistory[readingCounter % readingHistory.size()];
		
		/* Assign tail occurence pointer. Decrement ocurrence for previous value. */
		auto tailOccPtr = &occurencesTable[*headValPtr];
		if (*tailOccPtr > 0) --*tailOccPtr;
		
		/* Replace head value with new one. */
		*headValPtr = value;
		++occurencesTable[value];
		
		/* Increment readingCounter.  */
		++readingCounter;
	}

	uint16_t LineSensor::findModal() const
	{
		uint16_t modal{0};
		for (std::size_t i{0}; i < occurencesTable.size(); i++)
		{
			if (occurencesTable[i] > modal)
				modal = i;
		}

		return modal;
	}

	float LineSensor::calcValueRatio(uint16_t value) const
	{
		return value / static_cast<float>(findModal());
	}

	bool LineSensor::triggered()
	{
		if (occurencesTable.empty()) 
			return false;
		
		auto dacValue{static_cast<uint16_t>(analogRead(pin))};
		pushReading(dacValue);

		switch (currentStage)
		{
			case LSStage::CollectInitialValues:
			{
				/* Simply wait until readingCounter reaches required history size. */
				if (readingCounter > readingHistory.size())
					currentStage = LSMeasurementStage::WaitForStabilization;
			}
			break;

			case LSStage::WaitForStabilization:
			{
				/* If value is above treshold, then reset counter and wait agian. */
				if (calcValueRatio(dacValue) > RATIO_STABLE_TRESHOLD)
				{
					stabilizationCounter = 0; // TODO: verify on device
					break;
				}

				/* If value is below treshold as required, then wait for defined probes in a row. */
				if (++stabilizationCounter >= STABLE_PROBES_REQUIRED)
				{
					currentStage = LSStage::WaitForUphill;
					stabilizationCounter = 0;
				}
			}
			break;

			case LSStage::WaitForUphill:
			{
				/* 
					Simply wait for uphill. Then switch to waiting for stabilization again and
					return true to trigger behavior to be executed on detection.
				*/
				if (calcValueRatio(dacValue) > RATIO_UPHILL_TRESHOLD)
				{
					currentStage = LSStage::WaitForStabilization;
					return true;
				}
			}
			break;
		}

		return false;
	}
}