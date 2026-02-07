#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <iostream>
#include <format>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    this->tag = "insomnia_soni1541";
    
    this->server_uri = "test.mosquitto.org";
    
    this->client_id = this->tag;
    
    std::vector<InSomnia::Topic> input_topics =
    {
        { std::format("ntiurfu/{}/temp/inside",    this->tag), 1 },
        { std::format("ntiurfu/{}/temp/outside",   this->tag), 1 },
        { std::format("ntiurfu/{}/time",           this->tag), 1 },
        { std::format("ntiurfu/{}/current_lesson", this->tag), 1 }
    };
    
    int rc = -1;
    
    // Создание асинхронного клиента
    rc = MQTTAsync_create(
        &(this->client),
        this->server_uri.c_str(),
        this->client_id.c_str(),
        MQTTCLIENT_PERSISTENCE_NONE,
        nullptr);
    
    if (rc != MQTTASYNC_SUCCESS)
    {
        throw std::runtime_error(
            "Error create clinet");
    }
    
    this->callback_context.client = this->client;
    this->callback_context.connected = false;
    this->callback_context.topics = std::move(input_topics);
    
    // Установка callback-функций
    rc = MQTTAsync_setCallbacks(
        this->client,
        &(this->callback_context), 
        InSomnia::on_connection_lost,
        InSomnia::on_message,
        InSomnia::on_delivery_complete);
    
    if (rc != MQTTASYNC_SUCCESS) 
    {
        throw std::runtime_error(
            "Callbacks installation error");
    }
    
    // Настройка параметров подключения
    MQTTAsync_connectOptions conn_opts =
        MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.onSuccess = InSomnia::on_connect;
    conn_opts.onFailure = InSomnia::on_connect_failure;
    conn_opts.context = &(this->callback_context);
    
    // Асинхронное подключение к брокеру
    std::cout << std::format(
        "Connecting to the broker {}...\n",
        this->server_uri);
    std::cout.flush();
    
    rc = MQTTAsync_connect(this->client, &conn_opts);
    
    if (rc != MQTTASYNC_SUCCESS)
    {
        throw std::runtime_error(std::format(
            "Connection initialization error: {}", rc));
        
        MQTTAsync_destroy(&(this->client));
    }
    
}

MainWindow::~MainWindow()
{
    // Отключение от брокера
    if (this->callback_context.connected)
    {
        MQTTAsync_disconnectOptions disc_opts =
            MQTTAsync_disconnectOptions_initializer;
        
        disc_opts.onSuccess = InSomnia::on_disconnect;
        // disc_opts.context = this->client;
        disc_opts.context = &(this->callback_context);
        
        const int rc =
            MQTTAsync_disconnect(this->client, &disc_opts);
        if (rc != MQTTASYNC_SUCCESS)
        {
            std::cout << std::format(
                "Shutdown initialization error: {}\n", rc);
        }
        else
        {
            std::cout <<
                "Disconnection from the broker was successful\n";
        }
        std::cout.flush();
        
        // Даем время на завершение отключения
        sleep(1);
    }
    
    // Очистка ресурсов
    if (this->client)
    {
        MQTTAsync_destroy(&(this->client));
    }
    
    delete ui;
}
