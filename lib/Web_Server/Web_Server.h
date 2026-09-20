#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <Arduino.h>
#include <WebServer.h>

#include "datatypes.h"

/*
 * Initialize the local web server.
 *
 * The server reads the supplied DeviceStatus structure
 * and exposes it through the dashboard and REST API.
 */
bool WebServer_begin(DeviceStatus &device);

/*
 * Process incoming HTTP requests.
 *
 * Call repeatedly from loop().
 */
void WebServer_update();

/*
 * Returns true if the web server has been initialized.
 */
bool WebServer_isRunning();

#endif // __WEB_SERVER_H__