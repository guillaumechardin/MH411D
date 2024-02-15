/**
 * A nice library that handle MH410D/MH411D sensors from winsen
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


int MH411D::begin(Stream * serial, int scale)
{
  _serial = serial;
  MH411D::setScale(scale);
  return 1;
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
	//request co2 concentration value  from sensor command 0x86
	uint8_t request[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00};
    request[8] = getChecksum(request);
	//ask sensor to get co2 data
	MH411D::sendSensorData(request, 9);	
	
	// process sensor reply
	return MH411D::getSensorData();
}

uint8_t MH411D::getChecksum(uint8_t *data)
{
	uint8_t  checksum;
	for( int i = 1; i < 8; i++)
	{
		checksum += data[i];
	}
	
	checksum = 0xff - checksum;
	checksum += 1;
	#if DEBUG > 0
		Serial.print("Computed checksum:");
		Serial.println(checksum);
	#endif
	return checksum;
}

int MH411D::sendSensorData(uint8_t *data, uint8_t size)
{
	#if DEBUG > 0
		Serial.print("Send data on Serial:");
		Serial.println(data);
	#endif
	_serial->write(data, size);
	_serial->flush();
}

int MH411D::getSensorData()
{
	uint32_t startTime = millis();
	int i = 0;
	uint8_t incomingBytes[8];
	//limit to 9 bytes received as sensor message is 9 bytes.
	while (i < 9)
	{
		if ( _serial->available() )   //get uart data
		{
			incomingBytes[i] = _serial->read();
			i++;
		}
		else if (millis() - startTime > UARTTIMEOUTMS) //check if sensor replied in a short time
		{
			#if DEBUG > 0
				Serial.println("Data transfert tiemout");
			#endif
			return UARTTIMEOUTFAILED;
		}
		else
		{
			//NOP
		}
	}
	
	//all sensor data received
	#if DEBUG > 0
		Serial.println("Data acquisition complete");
		Serial.print("Data from sensor: ");
		Serial.println(incomingBytes, DEC);
	#endif

	if (incomingBytes[9] != getChecksum(incomingBytes) )
	{
		_co2 = -1;
		return CHECKSUMFAILED;
	}
	else
	{
		_co2 = computeGasConcentration(incomingBytes[2], incomingBytes[3]);
		return OPERATION_SUCCESS;
	}
}

void MH411D::setCalibrationZero()
{	
	#if DEBUG > 0
		Serial.println("Calibrating sensor method:Zero");
	#endif
	uint8_t request[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
	MH411D::sendSensorData(request, 9);	
}

uint8_t MH411D::computeGasConcentration(uint8_t byte2, uint8_t byte3)
{

	uint8_t value = 0;
	switch(_scale)
	{
		case UNITS_PPM:
			//Concentration high-position*256+Concentration low-position
			value = byte2 * 256 + byte3;
			break;
		case UNITS_PERCENT:
			//unit is %VOL)=(Concentration high-position*256+Concentration low-position/100
			value = (byte2 * 256 + byte3)/100;
			break;		
	}
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