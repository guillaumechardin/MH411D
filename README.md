# MH411D
A simple Arduino library to handle MH411D/MH410 sensors from winsen sensors.
Support measure and calibration Zero (this should be done in fresh air).


# Standard use


	#include <MH411D.h>
	MH411D myMH411D;

	void setup() {
	  // put your setup code here, to run once:
	  myMH411D.begin(&Serial1);
	  Serial1.begin(9600);

	}

	void loop() {
	  // put your main code here, to run repeatedly:
	  if( myMH411D.startMeasure() != 0)
	  {
		Serial.print('CO2 concentration: ');
		Serial.println(myMH411D.getCo2(), DEC);
	  }
	  else
	  {
		Serial.print('Measure failed !!');
	  }
	}
