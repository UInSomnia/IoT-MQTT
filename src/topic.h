#ifndef INSOMNIA_TOPIC_H
#define INSOMNIA_TOPIC_H

#include <string>
#include <cstdint>

namespace InSomnia
{
    class Topic
    {
    public:
        Topic();
        Topic(
            const std::string &path,
            const int QOS);
        
        void set_is_subscribe(
            const bool is_subscribe);
        
        const std::string& get_path() const;
        int get_QOS() const;
        bool get_is_subscribe() const;
        
    private:
        std::string path;
        int QOS;
        bool is_subscribe;
    };
}

#endif
