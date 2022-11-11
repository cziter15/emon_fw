/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the Energy Monitor firmware.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace apps::emon::components::utils
{
	enum class LSMeasurementStage
	{
		WAIT_PREWARM,
		WAIT_UPHILL,
		WAIT_STABILIZE
	};
	
	class LineSensor
	{
		private:
			static constexpr uint16_t EMON_STABILIZATION_PROBE_COUNT{10};			// How many stable values required to mark 'stable'.
			static constexpr float EMON_DEVIATION_UPHILL{1.75f};					// Deviation to detect 'rising' state.
			static constexpr float EMON_DEVIATION_STABILIZATION{1.1f};				// Max deviation to detect 'stable' state.

			LSMeasurementStage currentStage{LSMeasurementStage::WAIT_PREWARM};		// Current measurement stage.

			uint8_t pin{std::numeric_limits<uint8_t>::max()};						// Pin number.
			
			uint16_t readingCounter{0};												// Reading counter (will overflow).
			uint16_t stabilizationCounter{0};										// Counter of values for stabilization.

			std::vector<uint16_t> readingHistory;									// Last X value buffers for deviation calculation.
			std::vector<uint16_t> occurencesTable;									// Buffer for dominant calculation.

			void pushNewReading(uint16_t value);

			uint16_t calculateModal() const;
			float calculateDeviation(uint16_t value) const;

		public:
			/*
				Constructs LineSensor.

				@param probeCount Number of probes to calculate dominant.
				@param maxValue Max analog value.
				@param pin Analog pin number for the sensor.
			*/
			LineSensor(uint16_t probeCount, uint16_t maxValue, uint8_t pin);

			/*
				Handles all analog interpratation logic and returns if black line is detected.
				@return True if black line has been detected. Otherwise false.
			*/
			bool triggered();
	};
}