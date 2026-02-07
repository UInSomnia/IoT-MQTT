#include "mqtt_callback.h"

namespace InSomnia
{
    // Callback при успешном подключении
    void on_connect(
        void *context, MQTTAsync_successData *response)
    {
        std::cout << "Successful connection to the broker\n";
        std::cout.flush();
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = true;
        
        MQTTAsync client = callback_context->client;
        
        pack_subscribe(client, callback_context->topics);
    }
    
    void pack_subscribe(
        MQTTAsync client, std::vector<Topic> &topics)
    {
        for (Topic &topic : topics)
        {
            create_subscribe(client, topic);
        }
    }
    
    void create_subscribe(
        MQTTAsync client, Topic &topic)
    {
        MQTTAsync_responseOptions opts = 
            MQTTAsync_responseOptions_initializer;
        
        // Подписка на топик
        opts.onSuccess = on_subscribe;
        opts.onFailure = on_subscribe_failure;
        opts.context = &topic;
        
        const char *topik_path =
            topic.get_path().c_str();
        
        const int QOS = topic.get_QOS();
        
        const int rc = MQTTAsync_subscribe(
            client, topik_path, QOS, &opts);
        
        if (rc != MQTTASYNC_SUCCESS) 
        {
            throw std::runtime_error(
                std::format("Error when trying to subscribe", rc));
        }
    }
    
    // Callback при неудачном подключении
    void on_connect_failure(
        void *context, MQTTAsync_failureData *response)
    {
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        throw std::runtime_error(
            std::format(
                "Couldn't connect to the broker, code: {}",
                response ? response->code : 0));
    }
    
    // Callback при успешной подписке
    void on_subscribe(
        void *context, MQTTAsync_successData *response)
    {
        Topic *topic =
            reinterpret_cast<Topic*>(context);
        
        topic->set_is_subscribe(true);
        
        std::cout << std::format(
            "Successfully subscribed to the topic: {}\n",
            topic->get_path());
        std::cout.flush();
    }
    
    // Callback при неудачной подписке
    void on_subscribe_failure(
        void *context, MQTTAsync_failureData *response)
    {
        Topic *topic =
            reinterpret_cast<Topic*>(context);
        
        topic->set_is_subscribe(false);
        
        throw std::runtime_error(
            std::format(
                "Couldn't subscribed to the topic: {}, code: {}",
                topic->get_path(),
                response ? response->code : 0));
        
    }
    
    // Callback при получении сообщения
    int on_message(
        void *context,
        char *topic_name,
        int topic_len, 
        MQTTAsync_message *message)
    {
        std::string payload(
            static_cast<const char*>(message->payload),
            static_cast<size_t>(message->payloadlen)
        );
        
        std::cout << std::format(
            "A message was received asynchronously:\n"
            "+ Topic: {},\n"
            "+ Message: {},\n"
            "+ QoS: {}\n\n",
            topic_name, payload, message->qos);
        std::cout.flush();
        
        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topic_name);
        
        return 1;
    }
    
    // Callback при потере соединения
    void on_connection_lost(
        void *context, char *cause)
    {
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        printf("Соединение потеряно\n");
        if (cause)
            printf("Причина: %s\n", cause);
    }
    
    // Callback при отправке сообщения
    void on_delivery_complete(
        void *context, MQTTAsync_token token)
    {
        printf("Сообщение с токеном %d доставлено\n", token);
    }
    
    // Callback при отключении
    void on_disconnect(
        void *context, MQTTAsync_successData *response)
    {
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        printf("Успешно отключились от брокера\n");
    }
    
}
