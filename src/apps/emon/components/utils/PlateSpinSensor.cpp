/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "PlateSpinSensor.h"
#include <cmath>
#include <Arduino.h>

namespace apps::emon::components::utils
{
	PlateSpinSensor::PlateSpinSensor(uint8_t pin)
		: pin(pin)
	{
		readingHistory.resize(ADC_HISTORY_PROBES);
		occurencesTable.resize(MAX_ADC_VALUE);
		pinMode(pin, INPUT);
	}

	void PlateSpinSensor::pushReading(uint16_t value)
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

	uint16_t PlateSpinSensor::findModal() const
	{
		uint16_t modal{0};

		/* 
			Minimal analog reading is zero. To get modal simply return index which
			holds highest measured value in historical reading data.
		*/
		for (std::size_t i{0}; i < occurencesTable.size(); ++i)
		{
			if (occurencesTable[i] > modal)
				modal = i;
		}

		return modal;
	}

	float PlateSpinSensor::calcValueRatio(uint16_t value) const
	{
		return value / static_cast<float>(findModal());
	}

	bool PlateSpinSensor::triggered()
	{
		/* Do the work only i sensorTimer expired (otherwise skip and return false). */
		if (!sensorTimer.triggered())
			return false;
		
		/* Read analog value and push it to the history handling mechanism. */
		auto dacValue{static_cast<uint16_t>(analogRead(pin))};
		pushReading(dacValue);

		switch (currentStage)
		{
			case LSMStage::CollectInitialValues:
			{
				/* Simply wait until readingCounter reaches required history size. */
				if (readingCounter > readingHistory.size())
					currentStage = LSMStage::WaitForStabilization;
			}
			break;

			case LSMStage::WaitForStabilization:
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
					currentStage = LSMStage::WaitForUphill;
					stabilizationCounter = 0;
				}
			}
			break;

			case LSMStage::WaitForUphill:
			{
				/* 
					Simply wait for uphill. Then switch to waiting for stabilization again and
					return true to trigger behavior to be executed on detection.
				*/
				if (calcValueRatio(dacValue) > RATIO_UPHILL_TRESHOLD)
				{
					currentStage = LSMStage::WaitForStabilization;
					return true;
				}
			}
			break;
		}

		return false;
	}
}