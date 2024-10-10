/*
Copyright (c) 2020 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License 2.0
and Eclipse Distribution License v1.0 which accompany this distribution.

The Eclipse Public License is available at
   https://www.eclipse.org/legal/epl-2.0/
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.

SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause

Contributors:
   Roger Light - initial implementation and documentation.
*/

/*
 * This is an *example* plugin which demonstrates how to modify the payload of
 * a message after it is received by the broker and before it is sent on to
 * other clients.
 *
 * You should be very sure of what you are doing before making use of this feature.
 *
 * Compile with:
 *   gcc -I<path to mosquitto-repo/include> -fPIC -shared mosquitto_payload_modification.c -o mosquitto_payload_modification.so
 *
 * Use in config with:
 *
 *   plugin /path/to/mosquitto_payload_modification.so
 *
 * Note that this only works on Mosquitto 2.0 or later.
 */

// 1728599763: -----------  callback_message: topic=sensor-logger
// payload={"messageId":31,"sessionId":"b9af01e8-513f-42c7-9fb6-3c81fc23caf6",
//          "deviceId":"99dac4cb-d27e-4319-9e0e-5f9067ead1d7",
//          "payload":[{"name":"bluetooth","time":1728599762906000000,"values":{"id":"77d0b863-bdb9-e712-bb3b-c5b15959be20","rssi":-56,"manufacturerData":"99040510a855fdffff001c00d803c0be562c934cc26ed1702b44"}},{"name":"bluetooth","time":1728599763214000000,"values":{"id":"7d90872a-d4d8-566a-c8da-dd703caf2682","rssi":-59}}]}

#include <stdio.h>
#include <string.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"
#include "theengs.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

#define JSON_DOC_SIZE 4096

#define UNUSED(A) (void)(A)

#ifdef __cplusplus
extern "C" {
#endif
int mosquitto_plugin_version(int supported_version_count, const int *supported_versions);
int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count);
int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count);

#ifdef __cplusplus
} // extern "C"
#endif

static mosquitto_plugin_id_t *mosq_pid = NULL;

static void *theengs_decoder;
static const char *theengs_topic = "sensor-logger";

static int callback_message(int event, void *event_data, void *userdata)
{
    struct mosquitto_evt_message *ed =  static_cast<struct mosquitto_evt_message*>(event_data);

    UNUSED(event);
    UNUSED(userdata);

    // mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  callback_message: topic=%s payload=%.*s", ed->topic, ed->payloadlen, ed->payload);

    if ((ed->topic == NULL) || strcmp(ed->topic, theengs_topic)) { // no match, leave untouched
        return MOSQ_ERR_SUCCESS;
    }
    StaticJsonDocument<JSON_DOC_SIZE> doc;

    // parse payload as JSON
    DeserializationError err = deserializeJson(doc, ed->payload, ed->payloadlen);
    switch (err.code()) {
    case DeserializationError::Ok:
        // mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  parsed OK");
        break;
    default:
        mosquitto_log_printf( MOSQ_LOG_NOTICE, "Deserialization error: %s", err.c_str());
        return MOSQ_ERR_SUCCESS;
        break;
    }

    JsonArray jpl = doc["payload"].as<JsonArray>(); // ["values"]["manufacturerData"];

    for (JsonVariant item : jpl) {
        JsonVariant value = item["values"]["manufacturerData"];
        if (value)
            mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  JsonVariant: %s", value.as<const char*>());
    }

    // mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  MFD: %s", mfd);

    // iterate payload as p
    //      if p.haskey(manufacturerData) : p[theengs] = decodeResult(mfd)
    // sertialize
    // pass back

    // flatten option?

    return MOSQ_ERR_SUCCESS;
#if 0
    char *new_payload;
    uint32_t new_payloadlen;

    /* This simply adds "hello " to the front of every payload. You can of
     * course do much more complicated message processing if needed. */

    /* Calculate the length of our new payload */
    new_payloadlen = ed->payloadlen + (uint32_t)strlen("hello ")+1;

    /* Allocate some memory - use
     * mosquitto_calloc/mosquitto_malloc/mosquitto_strdup when allocating, to
     * allow the broker to track memory usage */
    new_payload = mosquitto_calloc(1, new_payloadlen);
    if(new_payload == NULL) {
        return MOSQ_ERR_NOMEM;
    }

    /* Print "hello " to the payload */
    snprintf(new_payload, new_payloadlen, "hello ");
    memcpy(new_payload+(uint32_t)strlen("hello "), ed->payload, ed->payloadlen);

    /* Assign the new payload and payloadlen to the event data structure. You
     * must *not* free the original payload, it will be handled by the
     * broker. */
    ed->payload = new_payload;
    ed->payloadlen = new_payloadlen;

    return MOSQ_ERR_SUCCESS;
#endif
}

int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
    int i;

    for(i=0; i<supported_version_count; i++) {
        if(supported_versions[i] == 5) {
            return 5;
        }
    }
    return -1;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
    UNUSED(user_data);
    UNUSED(opts);
    UNUSED(opt_count);

    theengs_decoder = Theengs_NewDecoder();

    mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  mosquitto_plugin_init decoder=%p", theengs_decoder);

    for(int i = 0; i < opt_count; i++) {
        mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  option '%d': '%s' = '%s'", i, opts[i].key, opts[i].value);
    }

    mosq_pid = identifier;
    return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, NULL);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
    UNUSED(user_data);
    UNUSED(opts);
    UNUSED(opt_count);

    if (theengs_decoder) {
        Theengs_DestroyDecoder(theengs_decoder);
    }
    mosquitto_log_printf( MOSQ_LOG_NOTICE, "-----------  mosquitto_plugin_cleanup");

    return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL);
}

