/** \file gps.c
 *  \brief Grove devices support library 
 */

/**
\addtogroup Grove devices
@{
 **************************************************************************																					
 *                                OpenPicus                 www.openpicus.com
 *                                                            italian concept
 * 
 *            openSource wireless Platform for sensors and Internet of Things	
 * **************************************************************************
 *  FileName:        touch.c
 *  Dependencies:    OpenPicus libraries
 *  Module:          FlyPort WI-FI - FlyPort ETH
 *  Compiler:        Microchip C30 v3.12 or higher
 *
 *  Author               Rev.    Date              Comment
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Davide Vicca	     1.0     17/04/2013		   First release  
 *  
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  Software License Agreement
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  This is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License (version 2) as published by 
 *  the Free Software Foundation AND MODIFIED BY OpenPicus team.
 *  
 *  ***NOTE*** The exception to the GPL is included to allow you to distribute
 *  a combined work that includes OpenPicus code without being obliged to 
 *  provide the source code for proprietary components outside of the OpenPicus
 *  code. 
 *  OpenPicus software is distributed in the hope that it will be useful, but 
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details. 
 * 
 * 
 * Warranty
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * WE ARE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 **************************************************************************/

#include "taskFlyport.h"
#include "grovelib.h"
#include "gps.h"
#include "nmea.h"
#include "stdio.h"
#include "math.h"


struct Interface *attachSensorToUartBus(void *,int,int,BYTE,BYTE);
	BYTE mon;

/**
 * struct Gps - Struct Gps Grove Sensor Device
 */

struct Gps
{
	const void *class;
	struct Interface *inter;
	BYTE uart_module;
	BYTE uart_interface;
    nmeaINFO info;
	nmeaPARSER parser;
	float lat;
	float lon;
	int year;
	BYTE mon;
	BYTE day;
	BYTE hour;
	BYTE min;
	BYTE sec;
	float speed;
};

/**
 * static void *Gps_ctor (void * _self, va_list *app)-Gps grove device Constructor  
 * \param *_self - pointer to the Gps grove device class.
 * \param *app 
 * \		1- uart module
* \return - Pointer to the Gps devices instantiated
*/
static void *Gps_ctor (void * _self, va_list *app)
{
	struct Gps *self = _self;
	self->uart_module =  va_arg(*app, BYTE);
	switch(self->uart_module)
	{
		case 2 :
		{
			self->uart_interface = 7;
			break;
		}
		case 3 :
		{
			self->uart_interface = 9;
			break;
		}
		case 4 :
		{
			self->uart_interface = 11;
			break;
		}			
	}
	self->inter = NULL;
	

	return self;
}	


/**
 * static void Gps_dtor (void * _sensor)- Gps grove device Destructor  
 * \param *_sensor - pointer to the Gps grove device class.
 * \return - None
*/
static void Gps_dtor (void * _sensor)
{
	struct Gps *sensor = _sensor;
	if(sensor->inter)
	{
		free(sensor->inter->port);
		free(sensor->inter);
	}
}	


/**
 * static void* Gps_attach (void * _board,void *_sensor,int n) - attach a Gps grove device to the GroveNest I2C port  
 * \param *_board - pointer to the GroveNest 
 * \param *_sensor - pointer to the Gps grove device class.
 * \param ic2bus -  which DIG port the device is connected to
 * \return 
 <UL>
	<LI><Breturn = Pointer to the DIG interface created:</B> the operation was successful.</LI> 
	<LI><B>return = NULL:</B> the operation was unsuccessful.</LI> 
 </UL>
 */
static void *Gps_attach (void * _board,void *_sensor,int port)
{
	struct Gps *sensor = _sensor;
	sensor->inter = attachSensorToUartBus(_board,port,9600,sensor->uart_module,sensor->uart_interface);	
	UARTOff(sensor->uart_module);
	return sensor->inter;
}	

/**
 *  static int Gps_set(void * _self, va_list *app) -  Perform a new gps acquisition.
 * \param *_self - pointer to the device 
 * \param *app - none 
 * \return:
 	<LI><Breturn = 0:</B>when the GPS device is connected on a Grove nest DIG port, with or without a valid connection </LI> 
 	<LI><Breturn = -1:</B>when the GPS device is not connected on a Grove nest DIG port. </LI> 
 </UL>
 */
static int Gps_set(void *_self,va_list *app)
{ 
	struct Gps *self = _self;
	char gps_sentences[70];
	unsigned int timer = 65000;
	flag = 1;
	nmea_zero_INFO(&self->info);
	nmea_parser_init(&self->parser);
	UARTOn(self->uart_module);
	UARTFlush(self->uart_module);
	vTaskDelay(50);
	while((UARTBufferSize(self->uart_module)<3) && timer)
		timer--;
	if(!timer)
		return -1;		
	timer = 500;
	while((gps_sentences[0] != '$') && timer)
	{
		timer--;
		while(UARTBufferSize(self->uart_module)<1);
		UARTRead(self->uart_module,gps_sentences,1);
		if(gps_sentences[0] == '$')
		{
			while(UARTBufferSize(self->uart_module) < 5);
			UARTRead(self->uart_module,(gps_sentences+1),5); //Reads all the chars in the
			if((gps_sentences[4] == 'M'))
			{
				while(UARTBufferSize(self->uart_module) < 65);					
				UARTRead(self->uart_module,(gps_sentences+6),64); //Reads all the chars in the
				gps_sentences[70] = '\0';
				int i = 0;
				i = nmea_parse(&self->parser, gps_sentences, (int)strlen(gps_sentences), &self->info);
				if(i)
				{
					flag = 0;
					self->lat = nmea_ndeg2degree (self->info.lat);
					self->lon = -nmea_ndeg2degree (self->info.lon);
					self->year = self->info.utc.year+1900;
					self->mon = self->info.utc.mon;
					self->day = self->info.utc.day;
					self->hour = self->info.utc.hour;
					self->min = self->info.utc.min;
					self->sec = self->info.utc.sec;
					self->speed = self->info.speed;
				    break;
				}
			}	
		}
		gps_sentences[0]= 0;
	}
	nmea_parser_destroy(&self->parser);
	UARTOff(self->uart_module);
	return 0;
}

/**
 * static float Gps_get(void * _self,va_list *app) -  Get the gps data.
 * \param *_self - pointer to the device 
 * \param *app - Which data to be got 
 *  - LATITUDE  - Provides the point's latitude 
 *  - LONGITUDE - Provides the point's longitude
 *  - YEAR 	- Provides the current year 
 *  - MONTH - Provides the current month 
 *  - DAY 	- Provides the current data 
 *  - HOUR	- Provides the current hour 
 *  - MINUTE	- Provides the current minute 
 *  - SECOND	- Provides the current second 
 *  - SPEED	- Provides the actual speed 
 *\return - The gps data chosen.
*/
static float Gps_get(void * _self, va_list *app)
{
	struct Gps *self = _self;
	BYTE param = va_arg(*app, BYTE);
	switch(param)
	{
		case LATITUDE:
			return self->lat;
		break;
		case LONGITUDE:
			return self->lon;
		break;
		case YEAR:
			return (float)self->year;
		break;
		case MONTH:
			return (float)self->mon;
		break;
		case DAY:
			return (float)self->day;
		break;
		case HOUR:
			return (float)self->hour;
		break;
		case MINUTE:
			return (float)self->min;
		break;
		case SECOND:
			return (float)self->sec;
		break;
		case SPEED:
			return self->speed;
		break;
		default:
			return 0;
	}
}

static const struct SensorClass _Gps =
{	
	sizeof(struct Gps),
	Gps_ctor,
	Gps_dtor,
	Gps_attach,
	0,
	Gps_set,
	Gps_get,
};

const void *Gps = &_Gps;

