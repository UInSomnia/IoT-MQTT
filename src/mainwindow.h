#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <string>
#include <memory>

#include <QMainWindow>

// #include "mqtt/async_client.h"
// #include "mqtt/topic.h"

extern "C"
{
#include <MQTTAsync.h>
}

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
    
private slots:
    void on_btn_connect_topics_clicked();
    
    void on_btn_connect_broker_clicked();
    
private:
    Ui::MainWindow *ui;
    
    // Строки
    std::string tag;
    std::string server_uri;
    std::string client_id;
    
    MQTTAsync client;
    
    MQTTAsync_connectOptions conn_opts;
    
    InSomnia::MQTT_Callback_Context callback_context;
    
    
    void slot_set_temp_inside(const std::string &message);
    
    void slot_set_temp_outside(const std::string &message);
    
    void slot_set_time(const std::string &message);
    
    void slot_set_current_lesson(const std::string &message);
};

#endif // MAINWINDOW_H
