/**
 * A nice library that handle MH410D/MH411D sensors from winsen sensors
 *
 * name     : MH411D
 * Author   : Guillaume CHARDIN
 * Version  : 0.0.1
 * Date     : 13/02/2024
 * url      : https://github.com/guillaumechardin/MH411D
 */
 
#include "MH411D.h"
 

MH411D::MH411D()
{
	int var = 0; 
}


void MH411D::begin(Stream * serial, int scale)
{
  _serial = serial;
  setScale(scale);
  //Serial.println("Begin init");
}


void MH411D::setScale(int scale)
{
  switch(scale)
  {
    case UNITS_PERCENT:
		_scale = UNITS_PERCENT;
		strcpy(_scaleDsp, "%");
		break;
    default:
		// not supported default to PPM
		_scale = UNITS_PPM;
		strcpy(_scaleDsp, "ppm");
  }
}

int MH411D::getCo2()
{
	return _co2;
}

int MH411D::startMeasure()
{
	#if DEBUG > 0
		Serial.println("Request sensor on CO2 concentration");
	#endif
	//request co2 concentration value  from sensor command 0x86
	uint8_t request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00};
    request[8] = getChecksum(request);
	//ask sensor to get co2 data
	sendSensorData(request, 9);	
	
	// process sensor reply
	return getSensorData();
}

uint8_t MH411D::getChecksum(uint8_t *data)
{
	#if DEBUG > 0
	Serial.println("Computing cheksum");
	for (int i=0;i<8;i++)
		{
			Serial.print(data[i],HEX);
			Serial.print(",");
		}
	#endif

	uint8_t  checksum;
	for( int i = 1; i < 8; i++)
	{
		checksum += data[i];
	}
	
	checksum = 0xff - checksum;
	checksum += 1;
	#if DEBUG > 0
		Serial.print("Checksum value:");
		Serial.println(checksum,HEX);
	#endif
	return checksum;
}

void MH411D::sendSensorData(uint8_t *data, uint8_t size)
{
	#if DEBUG > 0
		Serial.print("Send data on Serial:");
		for (int i=0; i<=8 ;i++)
		{
			Serial.print(data[i],HEX);
			Serial.print(",");
		}
		Serial.println("");
	#endif
	_serial->write(data, size);
	_serial->flush();
}

int MH411D::getSensorData()
{
	#if DEBUG > 0
		Serial.println("Get sensor data...");
	#endif

	uint32_t startTime = millis();
	int i = 0;
	uint8_t incomingBytes[9] = {0xff};
	//limit to 9 bytes received as sensor message is 9 bytes.
	while (i <= 8)
	{
		#if DEBUG > 0
			Serial.print("wait...");
			Serial.println(i);
		#endif

		if ( _serial->available() )   //get uart data
		{
			incomingBytes[i] = _serial->read();
			i++;
		}
		else if (millis() - startTime > UARTTIMEOUTMS) //check if sensor replied in a short time
		{
			#if DEBUG > 0
				Serial.println("Data transfert timeout");
			#endif
			return UARTTIMEOUTFAILED;
		}
		else
		{
			#if DEBUG > 0
				Serial.println("Still no data from serial...");
			#endif
			delay(10);
		}
	}
	
	//all sensor data received
	#if DEBUG > 0
		Serial.print("Data acquisition complete: ");
		for (int i=0; i<=8 ;i++)
		{
			//Serial.print(i,HEX);
			Serial.print(incomingBytes[i], HEX);
			Serial.print("_");
		}
		Serial.println("");
	#endif

	if (incomingBytes[8] != getChecksum(incomingBytes) )
	{
		#if DEBUG > 0
			Serial.println("Checksum error");
		#endif
		_co2 = -1;
		return CHECKSUMFAILED;
	}
	else
	{
		#if DEBUG > 0
			Serial.println("Checksum OK");
		#endif
		// no computation now 
		// _co2 = computeGasConcentration(incomingBytes[2], incomingBytes[3]);
		_co2 = (incomingBytes[3] * 256 + incomingBytes[4]);
		return OPERATION_SUCCESS;
	}
}

void MH411D::setCalibrationZeroPoint()
{	
	#if DEBUG > 0
		Serial.println("Calibrating sensor method:Zero");
	#endif
	uint8_t request[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
	sendSensorData(request, 9);	
}

void MH411D::setCalibrationSpanPoint(uint8_t spanHighPos, uint8_t spanLowPos)
{
	#if DEBUG > 0
		Serial.println("Calibrating sensor method:Span");
		Serial.print("Span high:");
		Serial.print(spanHighPos,DEC);
		Serial.print("Span2:");
		Serial.print(spanLowPos,DEC);
	#endif
	uint8_t request[9] = {0xFF, 0x01, 0x88, spanHighPos, spanLowPos, 0x00, 0x00, 0x00};
	request[8] = getChecksum(request);
	sendSensorData(request, 9);
}

uint8_t MH411D::computeGasConcentration(uint8_t byte2, uint8_t byte3)
{
	#if DEBUG > 0
		Serial.print("Computed data from sensor [unit:");
		Serial.print(_scaleDsp);
		Serial.println("]:");
		Serial.print("byte2:");
		Serial.print(byte2);
		Serial.print(" byte3:");
		Serial.println(byte3);
	#endif
	uint8_t value = 0;
	switch(_scale)
	{
		case UNITS_PERCENT:
			//unit is %VOL)=(Concentration high-position*256+Concentration low-position/100
			value = (byte2 * 256 + byte3);
			break;	
		
		default: //ppm
			//Concentration high-position*256+Concentration low-position
			value = byte2 * 256 + byte3;
			break;

	}
	#if DEBUG > 0
		Serial.print(value);
		Serial.print(" ");
		Serial.println(_scaleDsp);
	#endif
	return value;
}


int MH411D::getScale()
{
	return _scale;
}

char * MH411D::getScaleDsp()
{
	return _scaleDsp;
}