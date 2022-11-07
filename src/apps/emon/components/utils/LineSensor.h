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
			LSMeasurementStage currentMeasurementStage{LSMeasurementStage::WAIT_PREWARM};

			uint8_t pin{0};
			
			uint16_t stabilizationCounter{0};
			uint16_t lastValueIndex{0};

			std::vector<uint16_t> bufferedValues;
			std::vector<uint16_t> dominantBuffer;

		public:
			LineSensor(uint16_t probeCount, uint16_t maxValue, uint8_t pin);
			bool triggered();
	};
}