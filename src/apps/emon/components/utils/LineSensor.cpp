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

	void LineSensor::insertNewReading(uint16_t value)
	{
		/* Head value pointer assignment. */
		auto headValPtr = &readingHistory[iteration % BUFFER_SIZE];
		
		/* Assign tail occurence pointer. Decrement ocurrence for previous value. */
		auto tailOccPtr = &occurencesTable[*headValPtr];
		if (*tailOccPtr > 0) --*tailOccPtr;
		
		/* Replace head value with new one. */
		*headValPtr = value;
		++occurencesTable[value];
		
		/* Increment iteration count.  */
		++iteration;
	}

	uint16_t LineSensor::calculateModal() const
	{
		uint16_t modal{0};
		for (std::size_t i{0}; i < occurencesTable.size(); i++)
		{
			if (occurencesTable[i] > modal)
				modal = i;
		}

		return modal;
	}

	float LineSensor::calculateDeviation(uint16_t value) const
	{
		return value / static_cast<float>(calculateModal());
	}

	bool LineSensor::triggered()
	{
		if (occurencesTable.empty()) 
			return false;
		
		auto dacValue{static_cast<uint16_t>(analogRead(pin))};
		pushNewReading(dacValue);

		switch (currentStage)
		{
			case LSMeasurementStage::WAIT_PREWARM:
			{
				if (iteration > readingHistory.size())
					currentStage = LSMeasurementStage::WAIT_STABILIZE;
			}
			break;

			case LSMeasurementStage::WAIT_STABILIZE:
			{
				if (calculateDeviation(dacValue) <= EMON_DEVIATION_STABILIZATION)
				{
					if (++stabilizationCounter >= EMON_STABILIZATION_PROBE_COUNT)
					{
						currentStage = LSMeasurementStage::WAIT_UPHILL;
						stabilizationCounter = 0;
					}
				}
			}
			break;

			case LSMeasurementStage::WAIT_UPHILL:
			{
				if (calculateDeviation(dacValue) > EMON_DEVIATION_UPHILL)
				{
					currentStage = LSMeasurementStage::WAIT_STABILIZE;
					return true;
				}
			}
			break;
		}

		return false;
	}
}