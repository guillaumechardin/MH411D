#pragma once
/**
 * A nice library that handle MH410D/MH411D sensors from winsen sensors
 *
 * name     : MH411D
 * Author   : Guillaume CHARDIN
 * Version  : 0.0.1
 * Date     : 13/02/2024
 * url      : https://github.com/guillaumechardin/MH411D
 */

#ifndef MH411D_h
#define MH411D_h
#endif

#include "Arduino.h"
#include <string.h>

#define VERSION 				"0.0.1"
#define DEBUG 					1

#define UARTTIMEOUTMS 			5000 	//timeout millis

#define OPERATION_SUCCESS		1
#define OPERATION_FAILED        -1

#define UNITS_PPM				1
#define UNITS_PERCENT			2

#define GETCO2FAILED			-10
#define CHECKSUMFAILED			-11
#define UARTTIMEOUTFAILED		-20

#define BOOTUPDELAYMS 			5000




class MH411D
{
	public:
		//default constructor
		MH411D();
		
		/*
		 * Start sensor initialisation
		 * @param Stream A reference to access this already instanciated Serial Stream
		 * @param int scale to use either  UNITS_PPM | UNITS_PERCENT
		 * @return OPERATION_SUCCESS | OPERATION_FAILED
		 */
		void begin(Stream * serial, int scale = UNITS_PPM);
		
		/*
		 * getCo2 fetch and calculate CO2 concentration, return result in current units 
		 * [%] or [ppm] depending range default to ppm (see sensor datasheet for more informations)
		 * 
		 * @return int on success or GETCO2FAILED on failure
		 */
		int getCo2();
		
		/*
		 * Set sensor calibration level to 0ppm or 0% concentration volume.
		 * @return void
		 *
		 */
		void setCalibrationZeroPoint();
		
		/*
		 * Set sensor calibration level to a user defined span points
		 * @arg spanHighPos a ppm value (sure ?)
		 * @arg spanLowPos a ppm value (sure ?)
		 * @return void
		 */
		void setCalibrationSpanPoint(uint8_t spanHighPos, uint8_t spanLowPos);
		
		/*
		 * Start measure process on sensor.
		 * @return int operation status.  OPERATION_SUCCESS | UARTTIMEOUTFAILED | CHECKSUMFAILED 
		 *
		*/
		int startMeasure();
		
		/*
		 * Get current used scale for the running program
		 * @return int either UNITS_PPM | UNITS_PERCENT
		 */
		int getScale();

		/*
		 * Get current used scale unit display symbol 
		 * @return int pointer to char with ppm or % symbol
		 */
		char* getScaleDsp();

		/**
		 * 
		*/
		static const int BOOTUPDELAY = BOOTUPDELAYMS;
	
	protected:
		/*
		 * @var Stream pointer to an instanciated arduino Serial
		 */
		Stream *_serial = 0;
		
		
		/*
		 * @var int scale to be used   UNITS_PPM | UNITS_PERCENT
		 */
		int _scale = -1;
		
		
		/*
		 * @var int representing measure of co2 concentration
		 */
		int _co2 = -1;

		/*
		 * @var char representing units
		 */
		char _scaleDsp[3];
		
		/*
		 * Define sensor scale to be used. Either UNITS_PPM (for range 0-5000ppm) or UNITS_PERCENT (0-50%).
         * Avoid changing this during operations.
		 * See sensor datasheet for more details.
		 * @return void
		 */
		void setScale(int scale);
		
		/*
		 * Get data from sensor
		 * @param uint8_t 9 bytes array to store reply from serial
		 * @return int 
		 */
		int getSensorData();
		
		/*
		 * send data to sensor
		 * @param uint8_t 9 bytes array of data to send to sensor on serial
		 * @param int usually 9 as sensor way for 9 bytes
		 * @return void 
		 */
		void sendSensorData(uint8_t *data, uint8_t length);
		
		/*
		 * compute checksum from user request or sensor data
		 * @param pointer to uint8_t
		 * @return int
		 */
		uint8_t getChecksum(uint8_t * data);
		 
		 /*
		  * compute co2 concentration depending units
		  * @param  uint8_t Concentration high-position
		  * @param  uint8_t Concentration low-position
		  * @return uint8_t gas concentration
		  */
		uint8_t computeGasConcentration(uint8_t byte2, uint8_t byte3);
		
	
};