lib_grove_GPS_sensor
====================

Flyport library for Grove GPS sensor, released under GPL v.3.
The Grove GPS is powered via a E-1612-UB GPS module , can be used with u-center software, or directly be read GPS packet via serial port. The module series is a family of stand-alone GPS receivers featuring the high performance u-blox 5 positioning engine.
The communication with Flyport is realized through a specific library only needing few commands to make it works.<br>
An example of usage follows. More info on wiki.openpicus.com. 
1) import files inside Flyport IDE using the external libs button.
2) add following code example in FlyportTask.c:

<pre>
#include "taskFlyport.h"
#include "grovelib.h"
#include "gps.h"

char msg[200];

float lon;
float lat;
int year;
BYTE month;
BYTE day;
BYTE hour;
BYTE min;
BYTE sec;
float speed;

void FlyportTask()
{	
	vTaskDelay(100);
	UARTWrite(1,"Welcome to GROVE NEST example!\r\n");
 
	// GROVE board
	void *board = new(GroveNest);
 
	// GROVE devices	
	void *gps = new(Gps,2);//2 Uart module used
	attachToBoard(board,gps,DIG2);//Gps is connected to DIG2 port 
 
	UARTWrite(1,"GPS....\r\n");
	while(1)
	{
		vTaskDelay(200);		
		if(set(gps))//update the GPS data
			UARTWrite(1,"GPS not connected\n\r");			
		if(readError())	
			UARTWrite(1,"GPS data error\n\r");
		else
		{
			lon = get(gps,LONGITUDE);
			lat = get(gps,LATITUDE);
			year = get(gps,YEAR);
			month = get(gps,MONTH);
			day = get(gps,DAY);
			hour = get(gps,HOUR);
			min = get(gps,MINUTE);
			sec = get(gps,SECOND);
			speed = get(gps,SPEED);
			sprintf(msg,"lat %f,\n\rlong %f,\n\ryear %d,\n\rmonth %d,\
			\n\rday %d,\n\rhour %d,\n\rmin %d,\n\rsec %d\n\rspeed %f\n\r",
			(double)lat,(double)lon,year,month,day,hour,min,sec,(double)speed);
			UARTWrite(1,msg);
		}
	}
}
</pre>
