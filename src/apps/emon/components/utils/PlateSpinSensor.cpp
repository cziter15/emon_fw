/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#include "PlateSpinSensor.h"

namespace apps::emon::components::utils
{
	PlateSpinSensor::PlateSpinSensor(uint8_t pin)
		: pin(pin)
	{
		readingHistory.resize(ADC_HISTORY_PROBES);
		occurenceTable.resize(MAX_ADC_VALUE);
		pinMode(pin, INPUT);
	}

	void PlateSpinSensor::processNewProbe(uint16_t value)
	{
		/* Grab previous value and reference to previous occurence. */
		auto& prevValue{readingHistory[currentReadingIndex]};
		auto& prevValueOccurence{occurenceTable[prevValue]};

		/* Decrement occurence of the value that will be replaced by new reading. */
		if (prevValueOccurence > 0) 
			--prevValueOccurence;

		/* Replace the value and increment occurences. */
		prevValue = value;
		++occurenceTable[value];

		/* Increment reading index. */
		if (++currentReadingIndex >= readingHistory.size())
			currentReadingIndex = 0;
	}

	uint16_t PlateSpinSensor::findModal() const
	{
		uint16_t modal{0};

		/* 
			Minimal analog reading is zero. To get modal simply return index which
			holds highest measured value in historical reading data.
		*/
		for (std::size_t idx{0}; idx < occurenceTable.size(); ++idx)
			if (occurenceTable[idx] > modal)
				modal = idx;

		return modal;
	}

	float PlateSpinSensor::calcValueRatio(uint16_t value) const
	{
		auto modalValue{static_cast<float>(findModal())};
		return modalValue != 0.0f ? value / modalValue : 0.0f;
	}

	bool PlateSpinSensor::triggered()
	{
		/* Do the work only if interval passed (otherwise skip and return false). */
		if (!probeInterval.triggered())
			return false;
		
		/* Read analog value and process it for modal calculation mechanism. */
		auto adcValue{static_cast<uint16_t>(analogRead(pin))};
		processNewProbe(adcValue);

		/* Switch-based simple state machine. */
		switch (currentStage)
		{
			case LSMStage::CollectInitialValues:
			{
				/* This happens when processNewProbe has prepared initial buffer. */
				if (currentReadingIndex == 0)
					currentStage = LSMStage::WaitForStabilization;
			}
			break;

			case LSMStage::WaitForStabilization:
			{
				/* If value is above treshold, then reset counter and wait agian. */
				if (calcValueRatio(adcValue) > RATIO_STABLE_TRESHOLD)
				{
					stableProbesCount = 0;
					break;
				}

				/* If value is below treshold, then wait for defined probes in a row. */
				if (++stableProbesCount >= STABLE_PROBES_REQUIRED)
				{
					currentStage = LSMStage::WaitForUphill;
					stableProbesCount = 0;
				}
			}
			break;

			case LSMStage::WaitForUphill:
			{
				/* 
					Simply wait for uphill. Then switch to waiting for stabilization again and
					return true to trigger behavior to be executed on detection.
				*/
				if (calcValueRatio(adcValue) > RATIO_UPHILL_TRESHOLD)
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