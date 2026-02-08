#include "mqtt_callback.h"

namespace InSomnia
{
    // Callback при успешном подключении
    void on_connect(
        void *context, MQTTAsync_successData *response)
    {        
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = true;
        
        std::cout << "Successful connection to the broker\n";
        std::cout.flush();
        
        callback_context->callback_text_browser(
            "Подключение к брокеру завершено успешно");
        
        // MQTTAsync client = callback_context->client;
        // pack_subscribe(client, callback_context->topics);
    }
    
    // Callback при неудачном подключении
    void on_connect_failure(
        void *context, MQTTAsync_failureData *response)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        callback_context->callback_text_browser(
            "Не удалось установить соединение с брокером. "
            "Попробуйте повторить позже");
        
        // throw std::runtime_error(
        //     std::format(
        //         "Couldn't connect to the broker, code: {}",
        //         response ? response->code : 0));
    }
    
    // Callback при успешной подписке
    void on_subscribe(
        void *context, MQTTAsync_successData *response)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Subscribe_Context *subscribe_context =
            reinterpret_cast<MQTT_Subscribe_Context*>(context);
        
        Topic *topic =
            subscribe_context->topic;
        MQTT_Callback_Context *callback_context =
            subscribe_context->callback_context;
        
        delete subscribe_context;
        
        topic->set_is_subscribe(true);
        
        callback_context->callback_text_browser(
            std::format(
                "Подписка на топик оформлена ({})",
                topic->get_path()));
        
        std::cout << std::format(
            "Successfully subscribed to the topic: {}\n",
            topic->get_path());
        std::cout.flush();
    }
    
    // Callback при неудачной подписке
    void on_subscribe_failure(
        void *context, MQTTAsync_failureData *response)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Subscribe_Context *subscribe_context =
            reinterpret_cast<MQTT_Subscribe_Context*>(context);
        
        Topic *topic =
            subscribe_context->topic;
        MQTT_Callback_Context *callback_context =
            subscribe_context->callback_context;
        
        delete subscribe_context;
        
        topic->set_is_subscribe(false);
        
        callback_context->callback_text_browser(
            std::format(
                "Не удалось подписаться на топик ({})",
                topic->get_path()));
        
        // throw std::runtime_error(
        //     std::format(
        //         "Couldn't subscribed to the topic: {}, code: {}",
        //         topic->get_path(),
        //         response ? response->code : 0));
        
    }
    
    // Callback при получении сообщения
    int on_message(
        void *context,
        char *topic_name,
        int topic_len, 
        MQTTAsync_message *message)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        if (topic_name == nullptr)
        {
            throw std::runtime_error("Topic_name is null pointer");
        }
        if (message == nullptr)
        {
            throw std::runtime_error("Message is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        std::string payload(
            static_cast<const char*>(message->payload),
            static_cast<size_t>(message->payloadlen)
        );
        
        std::string str_topic_name(
            topic_name,
            topic_len);
        
        callback_context->callback_message_topic(
            str_topic_name, payload);
        
        std::cout << std::format(
            "A message was received asynchronously:\n"
            "+ Topic: {},\n"
            "+ Message: {},\n"
            "+ QoS: {}\n\n",
            str_topic_name, payload, message->qos);
        std::cout.flush();
        
        MQTTAsync_freeMessage(&message);
        MQTTAsync_free(topic_name);
        
        return 1;
    }
    
    // Callback при потере соединения
    void on_connection_lost(
        void *context, char *cause)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        std::string str_cause;
        if (cause)
        {
            str_cause = std::string(cause);
        }
        
        std::cout << std::format(
            "Connection lost.\n"
            "Cause: {}\n", str_cause);
        std::cout.flush();
    }
    
    // Callback при отправке сообщения
    void on_delivery_complete(
        void *context, MQTTAsync_token token)
    {
        // if (context == nullptr)
        // {
        //     throw std::runtime_error("Context is null pointer");
        // }
        
        std::cout << std::format(
            "Message with token {} "
            "delivery completed\n", token);
        std::cout.flush();
    }
    
    // Callback при отключении
    void on_disconnect(
        void *context, MQTTAsync_successData *response)
    {
        if (context == nullptr)
        {
            throw std::runtime_error("Context is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            reinterpret_cast<MQTT_Callback_Context*>(context);
        
        callback_context->connected = false;
        
        std::cout <<
            "Disconnected is successfull\n";
        std::cout.flush();
        
        // printf("Успешно отключились от брокера\n");
    }
    
    void pack_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context)
    {
        std::vector<Topic> &topics = callback_context.topics;
        
        for (Topic &topic : topics)
        {
            create_subscribe(
                client, callback_context, topic);
        }
    }
    
    void create_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context,
        Topic &topic)
    {
        if (client == nullptr)
        {
            throw std::runtime_error("Client is null pointer");
        }
        
        if (topic.get_is_subscribe())
        {
            return;
        }
        
        MQTTAsync_responseOptions opts = 
            MQTTAsync_responseOptions_initializer;
        
        MQTT_Subscribe_Context *subscribe_context =
            new MQTT_Subscribe_Context();
        
        subscribe_context->topic = &topic;
        subscribe_context->callback_context = &callback_context;
        
        // std::pair<MQTT_Callback_Context*, Topic*>
        //     *pair_callback_context_and_topic =
        //         new std::pair<MQTT_Callback_Context*, Topic*>(
        //             &callback_context, &topic);
        
        // Подписка на топик
        opts.onSuccess = on_subscribe;
        opts.onFailure = on_subscribe_failure;
        opts.context = subscribe_context;
        
        const char *topic_path =
            topic.get_path().c_str();
        
        const int QOS = topic.get_QOS();
        
        const int rc = MQTTAsync_subscribe(
            client, topic_path, QOS, &opts);
        
        if (rc != MQTTASYNC_SUCCESS) 
        {
            throw std::runtime_error(
                std::format("Error when trying to subscribe", rc));
        }
    }
    
}
