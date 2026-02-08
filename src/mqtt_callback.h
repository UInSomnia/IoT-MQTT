#ifndef INSOMNIA_MQTT_CALLBACK_H
#define INSOMNIA_MQTT_CALLBACK_H

// #include <memory>
#include <iostream>
#include <format>
#include <vector>
#include <functional>

#include "MQTTAsync.h"

#include "topic.h"

namespace InSomnia
{
    struct MQTT_Callback_Context
    {
        MQTTAsync client = nullptr;
        
        bool connected = false;
        
        std::vector<Topic> topics;
        
        std::function<void (const std::string)> callback_text_browser;
        
        std::function<void (const std::string, const std::string)> callback_message_topic;
    };
    
    struct MQTT_Subscribe_Context
    {
        Topic *topic = nullptr;
        
        MQTT_Callback_Context *callback_context = nullptr;
    };
    
    // Callback при успешном подключении
    // Context -> MQTT_Callback_Context
    void on_connect(
        void *context, MQTTAsync_successData *response);
    
    // Callback при неудачном подключении
    // Context -> MQTT_Callback_Context
    void on_connect_failure(
        void *context, MQTTAsync_failureData *response);
    
    // Callback при успешной подписке
    // Context -> MQTT_Subscribe_Context
    void on_subscribe(
        void *context, MQTTAsync_successData *response);
    
    // Callback при неудачной подписке
    // Context -> MQTT_Subscribe_Context
    void on_subscribe_failure(
        void *context, MQTTAsync_failureData *response);
    
    // Callback при получении сообщения
    // Context -> free
    int on_message(
        void *context,
        char *topic_name,
        int topic_len, 
        MQTTAsync_message *message);
    
    // Callback при потере соединения
    // Context -> MQTT_Callback_Context
    void on_connection_lost(
        void *context, char *cause);
    
    // Callback при отправке сообщения
    // Context -> free
    void on_delivery_complete(
        void *context, MQTTAsync_token token);
    
    // Callback при отключении
    // Context -> MQTT_Callback_Context
    void on_disconnect(
        void *context, MQTTAsync_successData *response);
    
    void pack_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context);
    
    void create_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context,
        Topic &topic);
    
}

#endif
