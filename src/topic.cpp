#include "topic.h"

#include <stdexcept>

namespace InSomnia
{

    Topic::Topic()
    {
        this->QOS = -1;
        this->is_subscribe = false;
    }
    
    Topic::Topic(
        const std::string &path,
        const int QOS,
        const std::function<void(const std::string &)>
            callback_set_value_to_ui)
    {
        this->path = path;
        this->QOS = QOS;
        this->is_subscribe = false;
        this->callback_set_value_to_ui = callback_set_value_to_ui;
    }
    
    void Topic::call_callback_set_value_to_ui(
        const std::string &message)
    {
        if (!(this->callback_set_value_to_ui))
        {
            throw std::runtime_error(
                "Callback_set_value_to_ui is null pointer");
        }
        
        this->callback_set_value_to_ui(message);
    }
    
    void Topic::set_is_subscribe(
        const bool is_subscribe)
    {
        this->is_subscribe = is_subscribe;
    }
    
    const std::string& Topic::get_path() const
    {
        return this->path;
    }
    
    int Topic::get_QOS() const
    {
        return this->QOS;
    }
    
    bool Topic::get_is_subscribe() const
    {
        return this->is_subscribe;
    }
    
}
