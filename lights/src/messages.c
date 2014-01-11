#include <pebble.h>

#include "messages.h"

static void _message_callback(DictionaryIterator *iterator, void *context);

enum RequestType {
    REQUEST_WAKEUP, REQUEST_SET = 1, REQUEST_GET, REQUEST_GET_RESPONSE
}; 
enum RequestTags {
    TAG_REQUEST_TYPE = 1, TAG_LIGHT, TAG_ADDRESS, TAG_CALLBACK
};

void message_init()
{
    uint32_t size = dict_calc_buffer_size(4,sizeof(Light),sizeof(uint16_t),sizeof(Light *),sizeof(GetLightCallback));
    app_message_register_inbox_received(_message_callback);
    app_message_open(size,size);
}

void _message_callback(DictionaryIterator *iterator, void *context)
{
    Tuple *tup = dict_find(iterator, TAG_REQUEST_TYPE);
    if (!tup) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, "unable to find request");
    }
    if (tup->value->uint16 == REQUEST_WAKEUP) {
        app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "got wakeup from app");
        return;
    } else if (tup->value->uint16 != REQUEST_GET_RESPONSE) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, "unknown request type %d",tup->value->uint16);
        return;
    }

    tup = dict_find(iterator, TAG_ADDRESS);
    if (!tup) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, "No address in response");
        return;
    }
    Light *light = NULL;
    memcpy(&light, tup->value->data, sizeof(Light *));

    tup = dict_find(iterator, TAG_LIGHT);
    if (!tup) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, "No light in response");
        return;
    }
    memcpy(light, tup->value->data, sizeof(Light));

    tup = dict_find(iterator, TAG_CALLBACK);
    if (!tup) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, "No callback in response");
        return;
    }
    GetLightCallback callback = NULL;
    memcpy(&callback, tup->value->data, sizeof(GetLightCallback));
    callback(light);
}

uint8_t setLight(Light *light)
{
    DictionaryIterator *iter = NULL;

    AppMessageResult result = app_message_outbox_begin(&iter);
    if (result != APP_MSG_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result);
        return false;
    }
    DictionaryResult result2 = dict_write_data(iter, TAG_LIGHT, (uint8_t *)light, sizeof(Light));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send data, status %d", result2);
        return false;
    }
    uint16_t r = REQUEST_SET;
    result2 = dict_write_data(iter, TAG_REQUEST_TYPE, (uint8_t *)&r, sizeof(uint16_t));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result2);
        return false;
    }
    app_message_outbox_send();
    return true;
}

uint8_t getLight(Light *light, GetLightCallback callback)
{
    DictionaryIterator *iter = NULL;
    AppMessageResult result = app_message_outbox_begin(&iter);
    if (result != APP_MSG_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result);
        return false;
    }
    DictionaryResult result2 = dict_write_data(iter, TAG_LIGHT, (uint8_t *)light, sizeof(Light));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send data, status %d", result2);
        return false;
    }
    uint16_t r = REQUEST_GET;
    result2 = dict_write_data(iter, TAG_REQUEST_TYPE, (uint8_t *)&r, sizeof(uint16_t));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result2);
        return false;
    }
    result2 = dict_write_data(iter, TAG_ADDRESS, (uint8_t *)&light, sizeof(Light *));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result2);
        return false;
    }
    result2 = dict_write_data(iter, TAG_CALLBACK, (uint8_t *)&callback, sizeof(GetLightCallback));
    if (result2 != DICT_OK) {
        app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, ",unable to send request, status %d", result2);
        return false;
    }

    app_message_outbox_send();
    return true;
}
