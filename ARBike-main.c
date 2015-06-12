/*
 *  Copyright (C) 2014 ThingWorx Inc.
 *
 *  Test application
 */

#include "twOSPort.h"
#include "twLogger.h"
#include "twApi.h"

#include <stdio.h>
#include <string.h>
#include "wiringSerial.h"

/* Name of our thing */
char * thingName = "ARBike";

int serialHandle = 0;

/*****************
A simple structure to handle
properties. Not related to
the API in anyway, just for
the demo application.
******************/
struct  {
	double FrontWheelSpeed;
	double FrontShockDisplacement;
	double SteeringAngle;
	double PedalsSpeed;
	double RearWheelSpeed;
	double SuspensionLinkAngle1;
	double Roll;
	double Pitch;
	double Heading;
	double AccelerationX;
	double AccelerationY;
	double AccelerationZ;
	double GyroX;
	double GyroY;
	double GyroZ;
	double Temperature;
} properties;

/*****************
Helper Functions
*****************/
void sendPropertyUpdate() {

	/* Create the property list */
	propertyList * proplist = twApi_CreatePropertyList("FrontWheelSpeed",twPrimitive_CreateFromNumber(properties.FrontWheelSpeed), 0);
	if (!proplist) {
		TW_LOG(TW_ERROR,"sendPropertyUpdate: Error allocating property list");
		return;
	}
	twApi_AddPropertyToList(proplist,"FrontShockDisplacement",twPrimitive_CreateFromNumber(properties.FrontShockDisplacement), 0);
	twApi_AddPropertyToList(proplist,"SteeringAngle",twPrimitive_CreateFromNumber(properties.SteeringAngle), 0);
	twApi_AddPropertyToList(proplist,"PedalsSpeed",twPrimitive_CreateFromNumber(properties.PedalsSpeed), 0);
	twApi_AddPropertyToList(proplist,"RearWheelSpeed",twPrimitive_CreateFromNumber(properties.RearWheelSpeed), 0);
	twApi_AddPropertyToList(proplist,"SuspensionLinkAngle1",twPrimitive_CreateFromNumber(properties.SuspensionLinkAngle1), 0);
	twApi_AddPropertyToList(proplist,"Roll",twPrimitive_CreateFromNumber(properties.Roll), 0);
	twApi_AddPropertyToList(proplist,"Pitch",twPrimitive_CreateFromNumber(properties.Pitch), 0);
	twApi_AddPropertyToList(proplist,"Heading",twPrimitive_CreateFromNumber(properties.Heading), 0);
	twApi_AddPropertyToList(proplist,"AccelerationX",twPrimitive_CreateFromNumber(properties.AccelerationX), 0);
	twApi_AddPropertyToList(proplist,"AccelerationY",twPrimitive_CreateFromNumber(properties.AccelerationY), 0);
	twApi_AddPropertyToList(proplist,"AccelerationZ",twPrimitive_CreateFromNumber(properties.AccelerationZ), 0);
	twApi_AddPropertyToList(proplist,"GyroX",twPrimitive_CreateFromNumber(properties.GyroX), 0);
	twApi_AddPropertyToList(proplist,"GyroY",twPrimitive_CreateFromNumber(properties.GyroY), 0);
	twApi_AddPropertyToList(proplist,"GyroZ",twPrimitive_CreateFromNumber(properties.GyroZ), 0);
	twApi_AddPropertyToList(proplist,"Temperature",twPrimitive_CreateFromNumber(properties.Temperature), 0);
	
	twApi_PushProperties(TW_THING, thingName, proplist, -1, FALSE);
	twApi_DeletePropertyList(proplist);
}

void shutdownTask(DATETIME now, void * params) {
	TW_LOG(TW_FORCE,"shutdownTask - Shutdown service called.  SYSTEM IS SHUTTING DOWN");
	twApi_UnbindThing(thingName);
	twSleepMsec(100);
	twApi_Delete();
	twLogger_Delete();
	exit(0);	
}

/***************
Data Collection Task
****************/
/*
This function gets called at the rate defined in the task creation.  The SDK has 
a simple cooperative multitasker, so the function cannot infinitely loop.
Use of a task like this is optional and not required in a multithreaded
environment where this functonality could be provided in a separate thread.
*/
#define DATA_COLLECTION_RATE_MSEC 2000
void dataCollectionTask(DATETIME now, void * params) {
    /* TW_LOG(TW_TRACE,"dataCollectionTask: Executing"); */
	
	serialPutchar (serialHandle, 's');            //Send 's' to the Arduberry
	char buffer[200] = {0};
	double prop_array[20]={0};	
	int i =0;
	while(serialDataAvail(serialHandle)){
		buffer[i]=  serialGetchar(serialHandle);        //Receive the characters from the Arduberry
		i++;		
	}
	i=0;
	char *cptr;
	
	cptr = strtok(buffer, " ");
	while( cptr!= NULL && i < 16)
	{
		prop_array[i]=strtod(cptr,NULL);
		cptr = strtok(NULL, " ");
		i++;
	}


	properties.FrontWheelSpeed = prop_array[0];
	properties.RearWheelSpeed = prop_array[1];
	properties.PedalsSpeed = prop_array[2];
	properties.SteeringAngle = prop_array[3];
	properties.SuspensionLinkAngle1 = prop_array[4];
	properties.FrontShockDisplacement = prop_array[5];
	properties.Roll = prop_array[6];
	properties.Pitch = prop_array[7];
	properties.Heading = prop_array[8];
	properties.AccelerationX = prop_array[9];
	properties.AccelerationY = prop_array[10];
	properties.AccelerationZ = prop_array[11];
	properties.GyroX = prop_array[12];
	properties.GyroY = prop_array[13];
	properties.GyroZ = prop_array[14];
	properties.Temperature = prop_array[15];
	
	/* Update the properties on the server */
	sendPropertyUpdate();
}

/*****************
Property Handler Callbacks 
******************/
enum msgCodeEnum propertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	char * asterisk = "*";
	if (!propertyName) propertyName = asterisk;
	TW_LOG(TW_TRACE,"propertyHandler - Function called for Entity %s, Property %s", entityName, propertyName);
	if (value) {
		if (isWrite && *value) {
			/* Property Writes */
/*			if (strcmp(propertyName, "InletValve") == 0) twInfoTable_GetBoolean(*value, propertyName, 0, &properties.InletValve); 
			}
			else return TWX_NOT_FOUND;
*/			return TWX_SUCCESS;
		} else {
			/* Property Reads */
			if (strcmp(propertyName, "FrontWheelSpeed") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.FrontWheelSpeed); 
			else if (strcmp(propertyName, "FrontShockDisplacement") == 0) *value = twInfoTable_CreateFromNumber(propertyName, 	properties.FrontShockDisplacement); 
			else if (strcmp(propertyName, "SteeringAngle") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.SteeringAngle); 
			else if (strcmp(propertyName, "PedalsSpeed") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.PedalsSpeed); 
			else if (strcmp(propertyName, "RearWheelSpeed") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.RearWheelSpeed); 
			else if (strcmp(propertyName, "SuspensionLinkAngle1") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.SuspensionLinkAngle1); 
			else if (strcmp(propertyName, "Roll") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Roll); 
			else if (strcmp(propertyName, "Pitch") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Pitch);
			else if (strcmp(propertyName, "Heading") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Heading);
			else if (strcmp(propertyName, "AccelerationX") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.AccelerationX);
			else if (strcmp(propertyName, "AccelerationY") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.AccelerationY);
			else if (strcmp(propertyName, "AccelerationZ") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.AccelerationZ);
			else if (strcmp(propertyName, "GyroX") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.GyroX);
			else if (strcmp(propertyName, "GyroY") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.GyroY);
			else if (strcmp(propertyName, "GyroZ") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.GyroZ);
			else if (strcmp(propertyName, "Temperature") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Temperature);
			else return TWX_NOT_FOUND;
		}
		return TWX_SUCCESS;
	} else {
		TW_LOG(TW_ERROR,"propertyHandler - NULL pointer for value");
		return TWX_BAD_REQUEST;
	}
}

/*****************
Service Callbacks 
******************/
/* Example of handling a single service in a callback */
/*enum msgCodeEnum addNumbersService(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	double a, b, res;
	TW_LOG(TW_TRACE,"addNumbersService - Function called");
	if (!params || !content) {
		TW_LOG(TW_ERROR,"addNumbersService - NULL params or content pointer");
		return TWX_BAD_REQUEST;
	}

	twInfoTable_GetNumber(params, "a", 0, &a);
	twInfoTable_GetNumber(params, "b", 0, &b);
	res = a + b;
	*content = twInfoTable_CreateFromNumber("result", res);
	if (*content) return TWX_SUCCESS;
	else return TWX_INTERNAL_SERVER_ERROR;
}
*/
/* Example of handling multiple services in a callback */
enum msgCodeEnum multiServiceHandler(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	TW_LOG(TW_TRACE,"multiServiceHandler - Function called");
	if (!content) {
		TW_LOG(TW_ERROR,"multiServiceHandler - NULL content pointer");
		return TWX_BAD_REQUEST;
	}
	if (strcmp(entityName, thingName) == 0) {
/*		if (strcmp(serviceName, "GetBigString") == 0) {
			int len = 10000;
			char text[] = "inna gadda davida ";
			char * bigString = (char *)TW_CALLOC(len,1);
			int textlen = strlen(text);
			while (bigString && len > textlen) {
				strncat(bigString, text, len - textlen - 1);
				len = len - textlen;
			}
			*content = twInfoTable_CreateFromString("result", bigString, FALSE);
			if (*content) return TWX_SUCCESS;
			return TWX_ENTITY_TOO_LARGE;

		} else */ if (strcmp(serviceName, "Shutdown") == 0) {
			/* Create a task to handle the shutdown so we can respond gracefully */
			twApi_CreateTask(1, shutdownTask);
		}
		return TWX_NOT_FOUND;	
	}
	return TWX_NOT_FOUND;	
}

/***************
Main Loop
****************/
/*
Solely used to instantiate and configure the API.
*/
int main( int argc, char** argv ) {

if(argc < 3) { 
printf("\nRequired arguments not found!\n\n");
printf("hostname(or IP) AppKey ScanRate\n");
printf("Example: ");
printf("ServerNameOrIP 	768252b8-1ace-4f6d-921c-ca20519b67dc 1000");
}

char* host=argv[1];
char* AppKey=argv[2];
int scanRate= atoi(argv[3]);

/*
printf("\nhost: %s\n", host);
printf("AppKey: %s\n", AppKey);
printf("scanRate: %d\n\n\n\n\n\n", scanRate);
*/	

// Initialize serial connection
serialHandle = serialOpen ("/dev/ttyAMA0",19200); //Open the Serial port
serialFlush(serialHandle);    //Clear the stream of old data packets


#if defined NO_TLS
	int16_t port = 80;
#else
	int16_t port = 443;
#endif

//	twDataShape * ds = 0;
	DATETIME nextDataCollectionTime = 0;
	int err = 0;

	twLogger_SetLevel(TW_TRACE);
	twLogger_SetIsVerbose(1);
	TW_LOG(TW_FORCE, "Starting up");

	/* Initialize Properties */
//	properties.Location.longitude = 42.3485868;
//	properties.Location.latitude = -71.07734189;

	/* Initialize the API */
	err = twApi_Initialize(host, port, TW_URI, AppKey, 
		NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	if (err) {
		TW_LOG(TW_ERROR, "Error initializing the API");
		exit(err);
	}

	/* Allow self signed certs */
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();

	/* Register our services */
/*	ds = twDataShape_Create(twDataShapeEntry_Create("a",NULL,TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("b",NULL,TW_NUMBER));
	twApi_RegisterService(TW_THING, thingName, "AddNumbers", NULL, ds, TW_NUMBER, NULL, addNumbersService, NULL);
	twApi_RegisterService(TW_THING, thingName, "GetBigString", NULL, NULL, TW_STRING, NULL, multiServiceHandler, NULL); */
	twApi_RegisterService(TW_THING, thingName, "Shutdown", NULL, NULL, TW_NOTHING, NULL, multiServiceHandler, NULL);

	/* Regsiter our properties */
	twApi_RegisterProperty(TW_THING, thingName, "FrontWheelSpeed", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "FrontShockDisplacement", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "SteeringAngle", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "PedalsSpeed", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "RearWheelSpeed", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "SuspensionLinkAngle1", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Roll", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Pitch", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Heading", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "AccelerationX", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "AccelerationY", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "AccelerationZ", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "GyroX", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "GyroY", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "GyroZ", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Temperature", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler, NULL);


	/* Bind our thing */
	twApi_BindThing(thingName);

	/* Connect to server */
	if (!twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES)) {
		/* Register our "Data collection Task" with the tasker */
		twApi_CreateTask(scanRate, dataCollectionTask);
	}

	while(1) {
	/*while (twApi_isConnected()) {*/
		char in = 0;
#ifndef ENABLE_TASKER
		DATETIME now = twGetSystemTime(TRUE);
		twApi_TaskerFunction(now, NULL);
		twMessageHandler_msgHandlerTask(now, NULL);
		if (twTimeGreaterThan(now, nextDataCollectionTime)) {
			dataCollectionTask(now, NULL);
			nextDataCollectionTime = twAddMilliseconds(now, scanRate);
		}
#else
		in = getch();
		if (in == 'q') break;
		else printf("\n");
#endif
		twSleepMsec(5);
	}
	twApi_UnbindThing(thingName);
	twSleepMsec(100);
	twApi_Delete();
	twLogger_Delete();
	exit(0);
}
