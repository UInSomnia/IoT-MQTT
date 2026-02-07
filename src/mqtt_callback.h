#ifndef INSOMNIA_MQTT_CALLBACK_H
#define INSOMNIA_MQTT_CALLBACK_H

// #include <memory>
#include <iostream>
#include <format>
#include <vector>

#include "MQTTAsync.h"

#include "topic.h"

namespace InSomnia
{
    struct MQTT_Callback_Context
    {
        MQTTAsync client = nullptr;
        
        bool connected = false;
        
        std::vector<Topic> topics;
    };
    
    // Callback при успешном подключении
    void on_connect(
        void *context, MQTTAsync_successData *response);
    
    void pack_subscribe(
        MQTTAsync client, std::vector<Topic> &topics);
    
    void create_subscribe(
        MQTTAsync client, Topic &topic);
    
    // Callback при неудачном подключении
    void on_connect_failure(
        void *context, MQTTAsync_failureData *response);
    
    // Callback при успешной подписке
    void on_subscribe(
        void *context, MQTTAsync_successData *response);
    
    // Callback при неудачной подписке
    void on_subscribe_failure(
        void *context, MQTTAsync_failureData *response);
    
    // Callback при получении сообщения
    int on_message(
        void *context,
        char *topic_name,
        int topic_len, 
        MQTTAsync_message *message);
    
    // Callback при потере соединения
    void on_connection_lost(
        void *context, char *cause);
    
    // Callback при отправке сообщения
    void on_delivery_complete(
        void *context, MQTTAsync_token token);
    
    // Callback при отключении
    void on_disconnect(
        void *context, MQTTAsync_successData *response);
}

#endif
