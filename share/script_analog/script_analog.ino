#include <stdlib.h>
#include <string.h>
#include <stdio.h>
int analog[8] = {A0,A1,A2,A3,A4,A5,A6,A7};
int hitavg[8] = {0,0,0,0,0,0,0,0};
int PinPlayTime[8] = {0,0,0,0,0,0,0,0};
int multiplex1[8];
int count = 0;
int total_playtime = 0;

void setup() {
  Serial.begin(115200);         
}

void loop() {
  readSensors();
  checkSensors();
}

void readSensors() {
  for(count=0; count <= 7; count++){
    multiplex1[count] = analogRead(analog[count]);
  }
}

void checkSensors() {
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

		if(v_max > 300)
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
