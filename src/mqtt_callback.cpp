#include "mqtt_callback.h"

#include <iostream>
#include <format>

namespace InSomnia
{
    MQTT_Callback_Context::MQTT_Callback_Context()
    {
        this->magic_control =
            MQTT_Callback_Context::magic_number;
        
        this->client = nullptr;
        
        this->is_connected = false;
        
        this->topic = nullptr;
    }
    
    MQTT_Callback_Context::MQTT_Callback_Context(
        MQTTAsync client,
        std::vector<Topic> &&topics,
        std::function<void (const std::string)> &&callback_text_browser)
    {
        this->magic_control =
            MQTT_Callback_Context::magic_number;
        
        this->client = client;
        this->is_connected = false;
        this->topics =
            std::make_shared<std::vector<Topic>>();
        *(this->topics) = std::move(topics);
        this->callback_text_browser =
            std::move(callback_text_browser);
        
        this->topic = nullptr;
        
        if (this->client == nullptr)
        {
            throw std::runtime_error(
                "Client is null pointer");
        }
    }
    
    MQTT_Callback_Context *MQTT_Callback_Context::
    from_context(void *context)
    {
        if (context == nullptr)
        {
            throw std::runtime_error(
                "Context is null pointer");
        }
        
        MQTT_Callback_Context *callback_context =
            static_cast<MQTT_Callback_Context*>(context);
        
        if (callback_context->magic_control !=
                MQTT_Callback_Context::magic_number)
        {
            throw std::runtime_error(std::format(
                "The MQTT_Callback_Context "
                "class object is corrupted: original {}, fact {}",
                MQTT_Callback_Context::magic_number,
                callback_context->magic_control));
        }
        
        return callback_context;
    }
    
    void MQTT_Callback_Context::outside_callback_text_browser(
        const std::string &message)
    {
        this->callback_text_browser(message);
    }
    
    void MQTT_Callback_Context::set_topic_for_subscribe(Topic *topic)
    {
        this->topic = topic;
    }
    
    bool MQTT_Callback_Context::get_is_connected() const
    {
        return this->is_connected;
    }
    
    const std::vector<Topic>& MQTT_Callback_Context::get_topics() const
    {
        return *(this->topics);
    }
    
    std::vector<Topic> &MQTT_Callback_Context::access_topics()
    {
        return *(this->topics);
    }
    
    // Callback при успешном подключении
    void MQTT_Callback_Context::internal_on_connect(
        MQTTAsync_successData *response)
    {
        this->is_connected = true;
        
        std::cout << "Successful connection to the broker\n";
        std::cout.flush();
        
        this->callback_text_browser(
            "Подключение к брокеру завершено успешно");
    }
    
    // Callback при неудачном подключении
    void MQTT_Callback_Context::internal_on_connect_failure(
        MQTTAsync_failureData *response)
    {
        this->is_connected = false;
        
        this->callback_text_browser(
            "Не удалось установить соединение с брокером. "
            "Попробуйте повторить позже");
    }
    
    // Callback при успешной подписке
    void MQTT_Callback_Context::internal_on_subscribe(
        MQTTAsync_successData *response)
    {
        if (this->topic == nullptr)
        {
            throw std::runtime_error("Topic is null pointer");
        }
        
        this->topic->set_is_subscribe(true);
        
        this->callback_text_browser(
            std::format(
                "Подписка на топик оформлена ({})",
                topic->get_path()));
        
        std::cout << std::format(
            "Successfully subscribed to the topic: {}\n",
            topic->get_path());
        
        std::cout.flush();
    }
    
    // Callback при неудачной подписке
    void MQTT_Callback_Context::internal_on_subscribe_failure(
        MQTTAsync_failureData *response)
    {
        if (this->topic == nullptr)
        {
            throw std::runtime_error("Topic is null pointer");
        }
        
        this->topic->set_is_subscribe(false);
        
        this->callback_text_browser(
            std::format(
                "Не удалось подписаться на топик ({})",
                topic->get_path()));        
    }
    
    // Callback при получении сообщения
    int MQTT_Callback_Context::internal_on_message(
        char *topic_name,
        int topic_len, 
        MQTTAsync_message *message)
    {
        if (topic_name == nullptr)
        {
            throw std::runtime_error("Topic_name is null pointer");
        }
        if (message == nullptr)
        {
            throw std::runtime_error("Message is null pointer");
        }
        
        std::string payload(
            static_cast<const char*>(message->payload),
            static_cast<size_t>(message->payloadlen)
        );
        
        std::string str_topic_name(
            topic_name,
            topic_len);
        
        InSomnia::Topic *selected_topic = nullptr;
        
        for (InSomnia::Topic &t : *(this->topics))
        {
            const std::string &current_path = t.get_path();
            if (str_topic_name == current_path)
            {
                selected_topic = &t;
            }
        }
        
        if (selected_topic == nullptr)
        {
            this->callback_text_browser(
                std::format(
                    "Не удалось найти топик ({}), "
                    "который прислал сообщение",
                    str_topic_name));
        }
        
        selected_topic
            ->call_callback_set_value_to_ui(payload);
        
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
    void MQTT_Callback_Context::internal_on_connection_lost(
        char *cause)
    {
        this->is_connected = false;
        
        std::string str_cause;
        if (cause)
        {
            str_cause = std::string(cause);
        }
        else
        {
            str_cause = std::string("Не известно");
        }
        
        this->callback_text_browser(
            std::format(
                "Соединение потеряно по причине: {}",
                str_cause));        
        
        std::cout << std::format(
            "Connection lost.\n"
            "Cause: {}\n", str_cause);
        std::cout.flush();
    }
    
    // Callback при отправке сообщения
    void MQTT_Callback_Context::internal_on_delivery_complete(
        MQTTAsync_token token)
    {
        std::cout << std::format(
            "Message with token {} "
            "delivery completed\n", token);
        std::cout.flush();
    }
    
    // Callback при отключении
    void MQTT_Callback_Context::internal_on_disconnect(
        MQTTAsync_successData *response)
    {
        this->is_connected = false;
        
        this->callback_text_browser(
            "Соединение успешно разорвано");  
        
        std::cout <<
            "Disconnected is successfull\n";
        std::cout.flush();
    }
    
    void on_connect(
        void *context, MQTTAsync_successData *response)
    {        
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_connect(response);
    }
    
    void on_connect_failure(
        void *context, MQTTAsync_failureData *response)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_connect_failure(response);
    }
    
    void on_subscribe(
        void *context, MQTTAsync_successData *response)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_subscribe(response);
        
        delete callback_context;
    }
    
    void on_subscribe_failure(
        void *context, MQTTAsync_failureData *response)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_subscribe_failure(response);
        
        delete callback_context;
    }
    
    int on_message(
        void *context,
        char *topic_name,
        int topic_len,
        MQTTAsync_message *message)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        const int value = callback_context->internal_on_message(
            topic_name, topic_len, message);
        
        return value;
    }
    
    void on_connection_lost(
        void *context, char *cause)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_connection_lost(cause);
    }
    
    void on_delivery_complete(
        void *context, MQTTAsync_token token)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_delivery_complete(token);
    }
    
    void on_disconnect(
        void *context, MQTTAsync_successData *response)
    {
        MQTT_Callback_Context *callback_context =
            MQTT_Callback_Context::from_context(context);
        
        callback_context->internal_on_disconnect(response);
    }
    
}
