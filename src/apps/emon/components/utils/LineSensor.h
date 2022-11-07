/*
 *	Copyright (c) 2021-2023, Krzysztof Strehlau
 *
 *	This file is part of the ksIotFramework library.
 *	All licensing information can be found inside LICENSE.md file.
 *
 *	https://github.com/cziter15/emon_fw/blob/master/LICENSE
 */

#pragma once

#include <vector>
#include <cstdint>
#include <limits>

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
			LSMeasurementStage currentStage{LSMeasurementStage::WAIT_PREWARM};		// Current measurement stage.

			uint8_t pin{std::numeric_limits<uint8_t>::max()};						// Pin number.
			
			uint16_t stabilizationCounter{0};										// Counter for stabilization values.
			uint16_t lastValueIndex{0};												// Last analog raw reading index.

			std::vector<uint16_t> bufferedValues;									// Last X value buffers for deviation calculation.
			std::vector<uint16_t> dominantBuffer;									// Buffer for dominant calculation.

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