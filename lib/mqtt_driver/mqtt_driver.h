#ifndef __MQTT_DRIVER__
#define __MQTT_DRIVER__
#include <Arduino.h>


#define MQTT_BROKER_ADDRESS "broker.hivemq.com"
#define MQTT_BROKER_PUBLISH_TOPIC "MKB0805_BloodPressure_EdgeAI_Machine"
#define MQTT_BROKER_SUBSCRIBE_TOPIC "MKB0805_BloodPressure_EdgeAI_Machine"




void mqtt_driver_init();





#endif //_MQTT_DRIVER