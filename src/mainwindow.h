#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <string>
#include <memory>

#include <QMainWindow>

// #include "mqtt/async_client.h"
// #include "mqtt/topic.h"

#include <MQTTAsync.h>

#include "topic.h"
#include "mqtt_callback.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// struct MQTTAsync_Destroy
// {
//     void operator () (MQTTAsync *client)
//     {
//         MQTTAsync_destroy(client);
//     }
// };

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    
    // Строки
    std::string tag;
    std::string server_uri;
    std::string client_id;

    // Векторы топиков
    // std::vector<InSomnia::Topic> input_topics;
    // std::vector<mqtt::topic_ptr> topics;
    
    MQTTAsync client;
    
    InSomnia::MQTT_Callback_Context callback_context;
    
    // MQTTAsync_connectOptions conn_opts;
    
    // MQTT клиент - объявлен последним, поэтому уничтожается первым
    // std::unique_ptr<mqtt::async_client> mqtt_client;
    // mqtt::async_client_ptr mqtt_client;
    
    // std::unique_ptr<mqtt::async_client, MqttClientDeleter> mqtt_client;
    // std::vector<std::pair<std::string, uint8_t>> input_topics;
    // std::vector<std::unique_ptr<mqtt::topic>> topics;
    
    // std::string tag;
    // std::string server_uri;
    // std::string client_id;
    
    // Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
