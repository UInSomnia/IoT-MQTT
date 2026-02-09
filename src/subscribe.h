#ifndef INSOMNIA_SUBSCRIBE_H
#define INSOMNIA_SUBSCRIBE_H

extern "C"
{
#include <MQTTAsync.h>
}

#include "mqtt_callback.h"

namespace InSomnia
{
    class Subscribe
    {
    public:
        Subscribe() = default;
        
        static void pack_subscribe(
            MQTTAsync client,
            MQTT_Callback_Context &callback_context);
        
    private:
        static void create_subscribe(
            MQTTAsync client,
            MQTT_Callback_Context &callback_context,
            Topic &topic);
        
    };
    
}

#endif
