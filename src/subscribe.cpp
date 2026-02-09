#include "subscribe.h"

#include <format>
#include <stdexcept>

namespace InSomnia
{

    void Subscribe::pack_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context)
    {
        std::vector<Topic> &topics =
            callback_context.access_topics();
        
        for (Topic &topic : topics)
        {
            create_subscribe(
                client, callback_context, topic);
        }
    }
    
    void Subscribe::create_subscribe(
        MQTTAsync client,
        MQTT_Callback_Context &callback_context,
        Topic &topic)
    {
        if (client == nullptr)
        {
            throw std::runtime_error("Client is null pointer");
        }
        
        // MQTT_Subscribe_Context *subscribe_context =
        //     new MQTT_Subscribe_Context();
        
        // subscribe_context->topic = &topic;
        // subscribe_context->callback_context = &callback_context;
        
        // std::pair<MQTT_Callback_Context*, Topic*>
        //     *pair_callback_context_and_topic =
        //         new std::pair<MQTT_Callback_Context*, Topic*>(
        //             &callback_context, &topic);
        
        if (topic.get_is_subscribe())
        {
            callback_context.outside_callback_text_browser(
                std::format(
                    "Подписка на топик ({}) уже была оформлена ранее",
                    topic.get_path()));
            return;
        }
        
        MQTT_Callback_Context *copy_callback_context =
            new MQTT_Callback_Context();
        *copy_callback_context = callback_context;
        
        copy_callback_context->set_topic_for_subscribe(&topic);
        
        // Подписка на топик
        MQTTAsync_responseOptions opts = 
            MQTTAsync_responseOptions_initializer;
        opts.onSuccess = on_subscribe;
        opts.onFailure = on_subscribe_failure;
        opts.context = copy_callback_context;
        
        const char *topic_path =
            topic.get_path().c_str();
        
        const int QOS = topic.get_QOS();
        
        const int rc = MQTTAsync_subscribe(
            client, topic_path, QOS, &opts);
        
        if (rc != MQTTASYNC_SUCCESS) 
        {
            throw std::runtime_error(std::format(
                "Error when trying to subscribe", rc));
        }
        
    }
    
}
