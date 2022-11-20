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
		/* Get a pointer to the value currently indexed. */
		auto indexedFieldPtr = &readingHistory[totalReadingCount % readingHistory.size()];
		
		/* Decrement occurence of the value that will be replaced by new reading. */
		auto oldValOccurrencePtr = &occurenceTable[*indexedFieldPtr];
		if (*oldValOccurrencePtr > 0) --*oldValOccurrencePtr;
		
		/* Replace the value and increment occurences. */
		*indexedFieldPtr = value;
		++occurenceTable[value];
		
		/* Increment reading count. */
		++totalReadingCount;
	}

	uint16_t PlateSpinSensor::findModal() const
	{
		uint16_t modal{0};

		/* 
			Minimal analog reading is zero. To get modal simply return index which
			holds highest measured value in historical reading data.
		*/
		for (std::size_t i{0}; i < occurenceTable.size(); ++i)
		{
			if (occurenceTable[i] > modal)
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
				/* Simply wait until reading count reaches required history size. */
				if (totalReadingCount > readingHistory.size())
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