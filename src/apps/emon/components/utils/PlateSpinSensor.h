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
	/* Plate Spin Sensor Measurement Stage enum. */
	enum class PSSMStage
	{
		/*
			In this state we wait for initial values to be collected.
			After that we should switch PSS measurement state to WaitForStabilization.
		*/
		CollectInitialValues,

		/*
			In this state we wait for trend stabilisation ('like flat line').
			After uphill detection the value should fall down and stabilize for a while.
			At least STABLE_PROBES_REQUIRED readings should be below RATIO_STABLE_TRESHOLD.
			In this case we should switch PSS measurement state to WaitForUphill.
		*/
		WaitForStabilization,

		/*
			In this step we wait for significant value increase. Significant increase
			means that the black part of the spinning plate is now right under the sensor.
			In this case we should switch PSS measurement state to WaitForStabilization.
		*/
		WaitForUphill
	};

	class PlateSpinSensor
	{
		private:
			static constexpr uint16_t ADC_HISTORY_PROBES{400};			// How many ADC readings to keep in history.
			static constexpr uint16_t MS_ADC_READ_INTERVAL{50};			// How often to read ADC (in ms).
			static constexpr uint16_t MAX_ADC_VALUE{1024};				// Max ADC value.
			
			static constexpr uint16_t STABLE_PROBES_REQUIRED{10};		// How many stable values required to mark trend as 'stable'.

			static constexpr float RATIO_UPHILL_TRESHOLD{2.0f};			// Treshold for trend uphill detection.
			static constexpr float RATIO_STABLE_TRESHOLD{1.1f};			// Treshold for trend stabilization.

			uint8_t pin{std::numeric_limits<uint8_t>::max()};			// Analog pin number for the sensor.

			PSSMStage currentStage{PSSMStage::CollectInitialValues};	// Current measurement stage.
			uint16_t currentReadingIndex{0};							// Current reading index in readingHistory.
			uint16_t stableProbesInARow{0};								// How many stable probes we have in a row.

			std::vector<uint16_t> readingHistory;						// Buffer for ADC readings history.
			std::vector<uint16_t> occurenceTable;						// Occurence table for readingHistory values.

			ksf::ksSimpleTimer probeInterval{MS_ADC_READ_INTERVAL};		// Timer for ADC reading interval.

			/*
				Processes new ADC reading and updates occurence table. 

				@param value New ADC reading.
			*/
			void processNewProbe(uint16_t value);

			/*
				Iterates over occcurences table most common value in reading history.

				@return Most common value in readingHistory.
			*/
			uint16_t findModal() const;

			/*
				Calculates ratio of passed value to the most common value in reading history.

				@param value Value to calculate ratio for.
				@return Ratio of passed value to the most common value in reading history.
			*/
			float calcValueRatio(uint16_t value) const;

		public:
			/*
				PlateSpinSensor constructor.

				@param pin Analog pin number for the sensor.
			*/
			PlateSpinSensor(uint8_t pin);

			/*
				Updates sensor state. Should be called in main loop.

				@return True if sensor detected black line. False otherwise.
			*/
			bool triggered();
	};
}