#include "topic.h"

namespace InSomnia
{

    Topic::Topic()
    {
        this->QOS = -1;
        this->is_subscribe = false;
    }
    
    Topic::Topic(const std::string &path, const int QOS)
    {
        this->path = path;
        this->QOS = QOS;
        this->is_subscribe = false;
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
