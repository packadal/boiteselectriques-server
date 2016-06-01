#include <stdlib.h>
#include <string.h>
#include <stdio.h>
int PinPlayTime[8] = {0,0,0,0,0,0,0,0};
int analogPin[1] = {A0};
int hitavg[8] = {0,0,0,0,0,0,0,0};
int r0 = 0;
int r1 = 0;
int r2 = 0;
int count = 0;
int multiplex1[8];
int total_playtime = 0;

//*******************************************************************************************************************
// Setup
//*******************************************************************************************************************

void setup()
{
	pinMode(2, OUTPUT);
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	Serial.begin(115200);                                  // connect to the serial port 115200
}

void loop()
{
	readSensors(0);
	checkSensors(0);
}

void readSensors (int analogPin)
{
	for(count=0; count <= 7; count++)
	{
		r2 = bitRead(count,0);
		r1 = bitRead(count,1);
		r0 = bitRead(count,2);
		digitalWrite(2, r0);
		digitalWrite(3, r1);
		digitalWrite(4, r2);

		multiplex1[count] = analogRead(analogPin);
	}
}

void checkSensors (int analogPin)
{
	const int num_iters = 1;
	if(total_playtime < num_iters)
	{
		for(int pin=0; pin <= 7; pin++)
		{
			hitavg[pin] += multiplex1[pin];
		}

		total_playtime++;
		return;
	}
	else
	{
		int v_max = 0;
		int i_max = 0;

		for(int pin=0; pin <= 7; pin++)
		{

			if(hitavg[pin] > v_max)
			{
				v_max = hitavg[pin] / num_iters;
				i_max = pin;
			}

			hitavg[pin] = 0;
			if(PinPlayTime[pin] > 0)
			{
				PinPlayTime[pin]--;
			}
		}

		if(v_max > 100)
		{
			if(PinPlayTime[i_max] == 0)
			{
				debug(i_max, v_max);
				PinPlayTime[i_max] = 1024;
			}
		}


		total_playtime = 0;
	}
}

void debug(int num, int vel)
{
	char message[32];
	sprintf(message, "%d %d\r\n", num, vel);
	Serial.write(message);
}
