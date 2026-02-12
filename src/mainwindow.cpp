#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>
#include <QMessageBox>

#include <iostream>
#include <format>

#include "subscribe.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    const std::string start_text = "нет данных";
    
    {
        QProgressBar *qpb_temp_inside =
            this->ui->progress_temp_inside;
        
        const int start_value =
            qpb_temp_inside->minimum();
        
        qpb_temp_inside->setValue(start_value);
        qpb_temp_inside->setFormat(start_text.c_str());
    }
    
    {
        QProgressBar *qpb_temp_outside =
            this->ui->progress_temp_outside;
        
        const int start_value = 
            qpb_temp_outside->minimum();
        
        qpb_temp_outside->setValue(start_value);
        qpb_temp_outside->setFormat(start_text.c_str());
    }
    
    this->color_cold = "#1E90FF";
    
    this->color_warm = "#00D700";
    
    this->tag = "insomnia_soni1541";
    
    this->server_uri = "test.mosquitto.org";
    
    this->client_id = this->tag;
    
    this->style_progress_cold =
        std::format(
            "QProgressBar {{"
            "    border: 1px solid #202020;"
            "    border-radius: 8px;"
            "    text-align: center;"
            "}} "
            "QProgressBar::chunk {{"
            "    background-color: {};"
            "    border-radius: 8px;"
            "}}", this->color_cold);
    
    this->style_progress_warm =
        std::format(
            "QProgressBar {{"
            "    border: 1px solid #202020;"
            "    border-radius: 8px;"
            "    text-align: center;"
            "}} "
            "QProgressBar::chunk {{"
            "    background-color: {};"
            "    border-radius: 8px;"
            "}}", this->color_warm);
    
    // std::cout << std::format(
    //     "Style cold: {}\n\n"
    //     "Style warm: {}\n\n",
    //     this->style_progress_cold,
    //     this->style_progress_warm);
    // std::cout.flush();
    
    std::vector<InSomnia::Topic> input_topics =
    {
        { std::format("ntiurfu/{}/temp/inside",    this->tag), 1,
          [this](const std::string &message){ this->slot_set_temp_inside(message); } },
        
        { std::format("ntiurfu/{}/temp/outside",   this->tag), 1,
          [this](const std::string &message){ this->slot_set_temp_outside(message); } },
        
        { std::format("ntiurfu/{}/time",           this->tag), 1,
          [this](const std::string &message){ this->slot_set_time(message); } },
        
        { std::format("ntiurfu/{}/current_lesson", this->tag), 1,
          [this](const std::string &message){ this->slot_set_current_lesson(message); } }
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
    
    // Наполняем callback_context
    this->callback_context =
        InSomnia::MQTT_Callback_Context(
            this->client,
            std::move(input_topics),
            [this](const std::string text) -> void
            {
                const QString qtext =
                    QString::fromStdString(text + "\n");
                this->ui->text_browser->append(qtext);
            });
    
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
    this->conn_opts =
        MQTTAsync_connectOptions_initializer;
    this->conn_opts.keepAliveInterval = 20;
    this->conn_opts.cleansession = 1;
    this->conn_opts.onSuccess = InSomnia::on_connect;
    this->conn_opts.onFailure = InSomnia::on_connect_failure;
    this->conn_opts.context = &(this->callback_context);
    
}

MainWindow::~MainWindow()
{
    // Отключение от брокера
    if (this->callback_context.get_is_connected())
    {
        MQTTAsync_disconnectOptions disc_opts =
            MQTTAsync_disconnectOptions_initializer;
        
        disc_opts.onSuccess = InSomnia::on_disconnect;
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
        // sleep(1);
    }
    
    // Очистка ресурсов
    if (this->client)
    {
        MQTTAsync_destroy(&(this->client));
    }
    
    delete ui;
}

void MainWindow::on_btn_connect_topics_clicked()
{
    if (this->callback_context.get_is_connected())
    {
        InSomnia::Subscribe::pack_subscribe(
            this->client,
            this->callback_context);
    }
    else
    {
        this->ui->text_browser->append(
            "Прежде чем подписываться на топики, "
            "необходимо установить соединение с брокером\n");
    }
}

void MainWindow::on_btn_connect_broker_clicked()
{
    if (this->callback_context.get_is_connected())
    {
        this->ui->text_browser->append(
            "Соединение с брокером уже установлено\n");
        return;
    }
    
    // Асинхронное подключение к брокеру
    std::cout << std::format(
        "Connecting to the broker {}...\n",
        this->server_uri);
    std::cout.flush();
    
    try
    {
        const int rc = MQTTAsync_connect(
            this->client, &(this->conn_opts));
        
        if (rc != MQTTASYNC_SUCCESS)
        {
            this->ui->text_browser->append(
                "Не удалось подключиться к брокеру. "
                "Пожалуйста, попробуйте ещё раз позже\n");
        }
    }
    catch (...)
    {
        this->ui->text_browser->append(
            "Не удалось подключиться к брокеру. "
            "Пожалуйста, попробуйте ещё раз позже\n");
    }
    
}

void MainWindow::slot_set_temp_inside(
    const std::string &message)
{
    const QString q_mes = QString::fromStdString(message);
    bool is_ok = false;
    const double d_val = q_mes.toDouble(&is_ok);
    if (!is_ok)
    {
        this->ui->text_browser->append(
            "Ошибка сигнала температуры внутри помещения\n");
        return;
    }
    const int i_val = static_cast<int>(d_val);
    this->ui->progress_temp_inside->setValue(i_val);
    
    const QString text =
        QString::fromStdString(
            std::format("{} °C", d_val));
    this->ui->progress_temp_inside->setFormat(text);
    
    const QString style_progress =
        (i_val < 0 ?
        this->style_progress_cold :
        this->style_progress_warm)
        .c_str();
    
    this
        ->ui
        ->progress_temp_inside
        ->setStyleSheet(style_progress);
}

void MainWindow::slot_set_temp_outside(
    const std::string &message)
{
    const QString q_mes = QString::fromStdString(message);
    bool is_ok = false;
    const double d_val = q_mes.toDouble(&is_ok);
    if (!is_ok)
    {
        this->ui->text_browser->append(
            "Ошибка сигнала температуры на улице\n");
        return;
    }
    const int i_val = static_cast<int>(d_val);
    this->ui->progress_temp_outside->setValue(i_val);
    
    const QString text =
        QString::fromStdString(
            std::format("{} °C", d_val));
    this->ui->progress_temp_outside->setFormat(text);
    
    const QString style_progress =
        (i_val < 0 ?
        this->style_progress_cold :
        this->style_progress_warm)
        .c_str();
    
    this
        ->ui
        ->progress_temp_outside
        ->setStyleSheet(style_progress);
}

void MainWindow::slot_set_time(
    const std::string &message)
{
    const QString q_mes = QString::fromStdString(message);
    QTime q_time;
    
    try
    {
        q_time = QTime::fromString(q_mes, "hh:mm:ss");
    }
    catch (...)
    {
        this->ui->text_browser->append(
            "Ошибка распознавания времени\n");
        return;
    }
    
    if (!q_time.isValid())
    {
        this->ui->text_browser->append(
            "Ошибка распознавания времени\n");
        return;
    }
    
    this->ui->time_set->setTime(q_time);
}

void MainWindow::slot_set_current_lesson(
    const std::string &message)
{
    const QString q_mes = QString::fromStdString(message);
    this->ui->label_current_lesson->setText(q_mes);
}
