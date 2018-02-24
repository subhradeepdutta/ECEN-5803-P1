/**----------------------------------------------------------------------------
 *
 *            \file freq.cpp
--                                                                           --
--              ECEN 5803 Mastering Embedded System Architecture             --
--                  Project 1 Module 4                                       --
--                Microcontroller Firmware                                   --
--                      freq.cpp                                             --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--                
--                
--  Designed by:  Tristan Lennertz, Subhradeep Dutta, & Omkar Prabhu 
-- 
-- Version: 2.1
-- Date of current revision:  2018-02
-- Target Microcontroller: Freescale MKL25ZVMT4 
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--               
--               
--   Functional Description:  
--   This file contains the functions and data structures needed to smooth 
--	 ADC samples and calculate frequency from the samples using a peak 
--	 detection algorithm. 
-- 
*/

#include <stdint.h>

/**
 * @brief Period between each ADC sample (and call of calculateFrequency)
 * Defined in seconds (float)
 */
#define SAMPLE_PERIOD (0.0001)

/**
 * @brief Size of the sliding window for moving average 
 */
#define WINDOW_SIZE (8)

/**
 * @brief Size of the sliding window for moving average 
 * Should be an odd number. 
 */
#define CIRCBUF_SIZE (9)

/**
 * @brief Sliding window buffer for moving average calculation of ADC samples
 */
static uint16_t adcWindowBuff[WINDOW_SIZE];

/**
 * @brief Circular buffer for peak detection storage of previous values
 */
static float peakBuff[CIRCBUF_SIZE]; 

/**
 * @brief Keeps track of the number of calls to the frequency detection function
 */  
static uint32_t samplesBeforePeak = 0; 

/**
 * @brief Internal store of the current best frequency estimate from algorithm
 */ 
static float currentFreqEstimate = 0; 

/* Internal function declarations */
float updateADCAvg(uint16_t latestValue);
uint8_t atPeak(float newAvg);
void updateFrequencyEstimate(float newFreq); 

/**
 * @brief Takes in the latest ADC sample and returns an updated frequency estimate
 * 
 * @param latestValue The latest value to be sampled from the ADC
 *
 * @return An updated frequency calculation taking into account the latest sample
 *
 * @note This function should only be called at a regular interval. More specifically,
 * it should be called at the same rate as the sampling of the ADC. This is define in
 * the SAMPLE_PERIOD constant, in seconds. 
 */  
uint32_t calculateFrequency(uint16_t latestValue)
{
	/* Smooth out the noise from the ADC with a moving average */
	float newAvg = updateADCAvg(latestValue);
	
	/* Update samples its been between peak detections */
	samplesBeforePeak++; 
	
	/* Check for a peak */
	if (atPeak(newAvg))
	{
		/* Extrapolate a frequency from this peak-to-peak period, update to
     * new current best estimate 
		 */
		float currentFreqEstimate = 1 / ((float)samplesBeforePeak * SAMPLE_PERIOD); 
		
		/* Reset running sample count */
		samplesBeforePeak = 0; 
	}
	
	return ((uint32_t) currentFreqEstimate); 
}

/**
 * @brief Takes in the latest ADC sample and returns an updated moving average. 
 *
 * Intended to smooth out the noise present in the ADC samples. Implemented using
 * a sliding windows moving average. 
 * 
 * @param latestValue The latest value to be sampled from the ADC
 *
 * @return An updated average value from the ADC
 */  
float updateADCAvg(uint16_t latestValue)
{
	static uint16_t buffIdx = 0;
	float retVal; 
	
	/* Index wraps around buffer, removing oldest value each time */
	buffIdx %= WINDOW_SIZE; 
	adcWindowBuff[buffIdx] = latestValue; 
	
	/* Perform calculation of the new average */
	int i, tempSum = 0;
	for (i = 0; i < WINDOW_SIZE; i++)
	{
		tempSum += adcWindowBuff[i];
	}
	retVal = (float)tempSum / (float)WINDOW_SIZE;
	
	/* Advace the buffer's index for next time */
	buffIdx++; 
		
	return retVal;
}

/**
 * @brief Takes in the latest ADC average value and determines whether a peak has 
 * been reached yet. 
 *
 * Keeps track of past inputs to this function to compare and determine whether a peak
 * has been reached. 
 *
 * @param latestValue The latest value to contribute to the peak comparison. 
 *
 * @return True if a peak is determined to exist, false otherwise. 
 */  
uint8_t atPeak(float newAvg)
{
	static uint16_t buffIdx = CIRCBUF_SIZE - 1; 
	uint16_t i, tempIdx; 
	float beforeAvg, afterAvg, centerVal; 
	
	/* Index wraps around buffer, removing oldest value as well as changing
   * the start of the comparison set each time	*/
	peakBuff[buffIdx] = newAvg; 
	
	/* The comparison set starts where the new value was added */
	tempIdx = buffIdx; 
	
	/* Average out the values that preceeded the center value */
	beforeAvg = 0; 
	for (i = 0; i < (CIRCBUF_SIZE / 2); i++)
	{
		/* Accumulate next value */ 
		beforeAvg += peakBuff[tempIdx++]; 
		
		/* Wrap around the buffer if needed */
		tempIdx %= CIRCBUF_SIZE; 
	}
	beforeAvg /= (CIRCBUF_SIZE / 2);
	
	/* Grab the center value */
	centerVal = peakBuff[tempIdx++];
	tempIdx %= CIRCBUF_SIZE; 
	
	/* Average out the values after the center value */
	afterAvg = 0; 
	for (i = 0; i < (CIRCBUF_SIZE / 2); i++)
	{
		/* Accumulate next value */ 
		afterAvg += peakBuff[tempIdx++]; 
		
		/* Wrap around the buffer if needed */
		tempIdx %= CIRCBUF_SIZE; 
	}
	afterAvg /= (CIRCBUF_SIZE / 2);
	
	/* Move the buffer's index for next time. Progresses backwards. */
	buffIdx = buffIdx ? buffIdx - 1 : CIRCBUF_SIZE - 1; 
	
	return (centerVal > beforeAvg && centerVal > afterAvg ? 1 : 0); 
}
