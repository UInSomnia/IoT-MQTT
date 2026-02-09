#ifndef INSOMNIA_MQTT_CALLBACK_H
#define INSOMNIA_MQTT_CALLBACK_H

#include <vector>
#include <functional>

extern "C"
{
#include <MQTTAsync.h>
}

#include "topic.h"

namespace InSomnia
{
    class MQTT_Callback_Context
    {
    public:
        MQTT_Callback_Context();
        
        MQTT_Callback_Context(
            MQTTAsync client,
            std::vector<Topic> &&topics,
            std::function<void (const std::string)> &&callback_text_browser);
        
        static MQTT_Callback_Context* from_context(void *context);
        
        // Callback при успешном подключении
        friend void on_connect(
            void *context, MQTTAsync_successData *response);
        
        // Callback при неудачном подключении
        friend void on_connect_failure(
            void *context, MQTTAsync_failureData *response);
        
        // Callback при успешной подписке
        friend void on_subscribe(
            void *context, MQTTAsync_successData *response);
        
        // Callback при неудачной подписке
        friend void on_subscribe_failure(
            void *context, MQTTAsync_failureData *response);
        
        // Callback при получении сообщения
        friend int on_message(
            void *context,
            char *topic_name,
            int topic_len, 
            MQTTAsync_message *message);
        
        // Callback при потере соединения
        friend void on_connection_lost(
            void *context, char *cause);
        
        // Callback при отправке сообщения
        friend void on_delivery_complete(
            void *context, MQTTAsync_token token);
        
        // Callback при отключении
        friend void on_disconnect(
            void *context, MQTTAsync_successData *response);
        
        void outside_callback_text_browser(
            const std::string &message);
        
        void set_topic_for_subscribe(Topic *topic);
        
        bool get_is_connected() const;
        
        const std::vector<Topic>& get_topics() const;
        
        std::vector<Topic>& access_topics();
        
    private:
        
        static constexpr uint32_t magic_number = 0xA00777FB;
        
        uint32_t magic_control;
        
        // Data
        
        MQTTAsync client;
        
        bool is_connected;
        
        std::vector<Topic> topics;
        
        std::function<void (const std::string)> callback_text_browser;
        
        // For subscribe
        Topic *topic;
        
        // Functions
        
        // Callback при успешном подключении
        // Context -> MQTT_Callback_Context
        void internal_on_connect(
            MQTTAsync_successData *response);
        
        // Callback при неудачном подключении
        // Context -> MQTT_Callback_Context
        void internal_on_connect_failure(
            MQTTAsync_failureData *response);
        
        // Callback при успешной подписке
        // Context -> MQTT_Subscribe_Context
        void internal_on_subscribe(
            MQTTAsync_successData *response);
        
        // Callback при неудачной подписке
        // Context -> MQTT_Subscribe_Context
        void internal_on_subscribe_failure(
            MQTTAsync_failureData *response);
        
        // Callback при получении сообщения
        // Context -> free
        int internal_on_message(
            char *topic_name,
            int topic_len, 
            MQTTAsync_message *message);
        
        // Callback при потере соединения
        // Context -> MQTT_Callback_Context
        void internal_on_connection_lost(
            char *cause);
        
        // Callback при отправке сообщения
        // Context -> free
        void internal_on_delivery_complete(
            MQTTAsync_token token);
        
        // Callback при отключении
        // Context -> MQTT_Callback_Context
        void internal_on_disconnect(
            MQTTAsync_successData *response);
        
    };
    
    // struct MQTT_Subscribe_Context
    // {
    //     Topic *topic = nullptr;
        
    //     MQTT_Callback_Context *callback_context = nullptr;
    // };
    
    // Callback при успешном подключении
    void on_connect(
        void *context, MQTTAsync_successData *response);
    
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
