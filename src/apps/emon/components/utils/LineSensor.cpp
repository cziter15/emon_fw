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
		bufferedValues.resize(probeCount);
		dominantBuffer.resize(maxValue);
		
		if (pin != std::numeric_limits<uint8_t>::max())
			pinMode(pin, INPUT);
	}

	bool LineSensor::triggered()
	{
		if (pin == std::numeric_limits<uint8_t>::max())
			return false;

		uint16_t dominant{0};
		auto anaValue{static_cast<uint16_t>(analogRead(pin))};

		bufferedValues[lastValueIndex++ % bufferedValues.size()] = anaValue;
		std::fill(std::begin(dominantBuffer), std::end(dominantBuffer), 0);

		for (std::size_t i{0}; i < bufferedValues.size(); i++)
		{
			uint16_t currentValue{bufferedValues[i]};

			if (currentValue < dominantBuffer.size()) 
				dominantBuffer[currentValue]++;
		}

		for (std::size_t i{0}; i < dominantBuffer.size(); i++)
		{
			if (dominantBuffer[i] > dominant)
				dominant = i;
		}

		float currentDeviation{static_cast<float>(anaValue) / static_cast<float>(dominant)};

		switch (currentStage)
		{
			case LSMeasurementStage::WAIT_PREWARM:
			{
				if (lastValueIndex > bufferedValues.size())
					currentStage = LSMeasurementStage::WAIT_STABILIZE;
			}
			break;

			case LSMeasurementStage::WAIT_STABILIZE:
			{
				if (fabsf(currentDeviation) <= EMON_DEVIATION_STABILIZATION)
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
				if (fabsf(currentDeviation) > EMON_DEVIATION_UPHILL)
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