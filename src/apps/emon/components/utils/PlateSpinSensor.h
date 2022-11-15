/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <ksIotFrameworkLib.h>

namespace apps::emon::components::utils
{
	enum class LSMStage
	{
		/*
			Ocurrence table must be filled with data before calculating modal
			value. Otherwise false positive results will be triggered.
		*/
		CollectInitialValues,

		/*
			In this state we wait for trend stabilisation ('like flat line').
			After uphill detection the value should fall down and stabilize for a while.
			At least STABLE_PROBES_REQUIRED readings should be below RATIO_STABLE_TRESHOLD.
			If this condition is met, we again should wait for uphill.
		*/
		WaitForStabilization,

		/*
			In this step we wait for significant value increase. Significant increase
			means that black part of the spinning plate is right under the sensor.
			In this case we should switch state to WaitForStabilization.
		*/
		WaitForUphill
	};

	class PlateSpinSensor
	{
		private:
			static constexpr uint16_t ADC_HISTORY_PROBES{400};				// How probes required to calculate dominant.
			static constexpr uint16_t MS_ADC_READ_INTERVAL{50};				// Fast timer interval to measure ADC value.
			static constexpr uint16_t MAX_ADC_VALUE{1024};					// This is in general MAX ADC value that can be read.
			
			static constexpr uint16_t STABLE_PROBES_REQUIRED{10};			// How many stable values required to mark trend as 'stable'.
			static constexpr float RATIO_UPHILL_TRESHOLD{1.75f};			// Ratio treshold to mark all values above as UPHILL.
			static constexpr float RATIO_STABLE_TRESHOLD{1.1f};				// Ratio treshold to mark all values below as STABLE.

			uint8_t pin{std::numeric_limits<uint8_t>::max()};				// Pin number.
			
			LSMStage currentStage{LSMStage::CollectInitialValues};			// Current measurement stage.
			uint16_t totalReadingCount{0};									// Reading counter (will overflow).
			uint16_t stableProbesCount{0};									// Counter of values for stabilization.

			std::vector<uint16_t> readingHistory;							// Reading history (max size defined in ctor).
			std::vector<uint16_t> occurenceTable;							// Buffer for modal value calculation.

			ksf::ksSimpleTimer probeInterval{MS_ADC_READ_INTERVAL};			// Timer for sensor ticking.

			/*
				Replaces a value in reading history (works like circular buffer).
				Handles counter increment/decremnt occurencesTable.

				@param value New reading value.
			*/
			void processNewProbe(uint16_t value);

			/*
				Iterates over occcurences table most common value in reading history.

				@return Most common value in readingHistory.
			*/
			uint16_t findModal() const;

			/*
				Calculates ratio of passed value to the most common value in reading history.

				@param value Input value.
				@return Calculated ratio (passed value) / (dominant in reading history).
			*/
			float calcValueRatio(uint16_t value) const;

		public:
			/*
				Constructs PlateSpinSensor.

				@param pin Analog pin number for the sensor.
			*/
			PlateSpinSensor(uint8_t pin);

			/*
				Handles all analog interpratation logic and returns if black line is detected.

				@return True if black line has been detected. Otherwise false.
			*/
			bool triggered();
	};
}