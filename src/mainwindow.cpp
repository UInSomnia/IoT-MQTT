#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>
#include <QMessageBox>
#include <QDateTime>

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
    
    this->ui->label_filler_time->setText(start_text.c_str());
    this->ui->label_filler_time->show();
    this->ui->time_set->hide();
    this->ui->time_set->setEnabled(false);
    
    this->ui->label_current_lesson->setText(start_text.c_str());
    this->ui->label_last_active->setText(start_text.c_str());
    
    std::vector<InSomnia::Topic> input_topics =
    {
        { std::format("ntiurfu/{}/temp/inside",    this->tag), 1,
          [this](const std::string &message){ this->slot_set_temp_inside(message); } },
        
        { std::format("ntiurfu/{}/temp/outside",   this->tag), 0,
          [this](const std::string &message){ this->slot_set_temp_outside(message); } },
        
        { std::format("ntiurfu/{}/time",           this->tag), 0,
          [this](const std::string &message){ this->slot_set_time(message); } },
        
        { std::format("ntiurfu/{}/current_lesson", this->tag), 2,
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
                
                // this->ui->text_browser->append(qtext);
                
                QMetaObject::invokeMethod(
                this,
                [this, qtext]()
                {
                    this->ui->text_browser->append(qtext);
                },
                Qt::QueuedConnection);
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
    
    this->topic_lesson = nullptr;
    this->topic_temp_inside = nullptr;
    this->topic_temp_outside = nullptr;
    this->topic_time = nullptr;
    
    this->extract_topics();
    
    this->ui->slider_temp_inside->setMaximum(50);
    this->ui->slider_temp_inside->setMinimum(-50);
    this->ui->slider_temp_inside->setValue(0);
    
    this->ui->slider_temp_outside->setMaximum(50);
    this->ui->slider_temp_outside->setMinimum(-50);
    this->ui->slider_temp_outside->setValue(0);
    
    this->ui->label_temp_inside->setText(
        QString::number(this->ui->slider_temp_inside->value()));
    this->ui->label_temp_outside->setText(
        QString::number(this->ui->slider_temp_outside->value()));
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
    // this->ui->progress_temp_inside->setValue(i_val);
    
    const QString text =
        QString::fromStdString(
            std::format("{} °C", d_val));
    // this->ui->progress_temp_inside->setFormat(text);
    
    const QString style_progress =
        (i_val < 0 ?
        this->style_progress_cold :
        this->style_progress_warm)
        .c_str();
    
    // this
    //     ->ui
    //     ->progress_temp_inside
    //     ->setStyleSheet(style_progress);
    
    QMetaObject::invokeMethod(
    this,
    [this, i_val, text, style_progress]()
    {
        this->ui->progress_temp_inside->setValue(i_val);
        this->ui->progress_temp_inside->setFormat(text);
        this
            ->ui
            ->progress_temp_inside
            ->setStyleSheet(style_progress);
        this->set_last_active();
    },
    Qt::QueuedConnection);
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
    // this->ui->progress_temp_outside->setValue(i_val);
    
    const QString text =
        QString::fromStdString(
            std::format("{} °C", d_val));
    // this->ui->progress_temp_outside->setFormat(text);
    
    const QString style_progress =
        (i_val < 0 ?
        this->style_progress_cold :
        this->style_progress_warm)
        .c_str();
    
    // this
    //     ->ui
    //     ->progress_temp_outside
    //     ->setStyleSheet(style_progress);
    
    QMetaObject::invokeMethod(
    this,
    [this, i_val, text, style_progress]()
    {
        this->ui->progress_temp_outside->setValue(i_val);
        this->ui->progress_temp_outside->setFormat(text);
        this
            ->ui
            ->progress_temp_outside
            ->setStyleSheet(style_progress);
        this->set_last_active();
    },
    Qt::QueuedConnection);
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
    
    // this->ui->time_set->setTime(q_time);
    
    // this->ui->label_filler_time->hide();
    // this->ui->time_set->show();
    
    QMetaObject::invokeMethod(
    this,
    [this, q_time]()
    {
        this->ui->time_set->setTime(q_time);
        this->ui->label_filler_time->hide();
        this->ui->time_set->show();
        this->set_last_active();
    },
    Qt::QueuedConnection);
}

void MainWindow::slot_set_current_lesson(
    const std::string &message)
{
    const QString q_mes = QString::fromStdString(message);
    
    // this->ui->label_current_lesson->setText(q_mes);
    
    QMetaObject::invokeMethod(
    this,
    [this, q_mes]()
    {
        this->ui->label_current_lesson->setText(q_mes);
        this->set_last_active();
    },
    Qt::QueuedConnection);
}

void MainWindow::set_last_active()
{
    const QDateTime current =
        QDateTime::currentDateTime();
    
    const QString text =
        current.toString("dd.MM.yyyy hh:mm:ss");
    
    this->ui->label_last_active->setText(text);
}

void MainWindow::setting_parametr(
    InSomnia::Topic *topic_set, const std::string &message)
{
    if (topic_set == nullptr)
    {
        throw std::runtime_error(
            "Topic set is null pointer");
    }
    
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    MQTTAsync_responseOptions opts = 
        MQTTAsync_responseOptions_initializer;
    
    // Настраиваем параметры отправки
    opts.onSuccess = InSomnia::on_publish_success;
    opts.onFailure = InSomnia::on_publish_failure;
    // opts.context = pubContext; // Передаем контекст в callback
    opts.context = &(this->callback_context);
    
    
    if(topic_set->get_is_subscribe() == false)
    {
        this->ui->text_browser->append(
            "Необходимо подписаться на топик\n");
        return;
    }
    
    // Настраиваем сообщение
    pubmsg.payload = const_cast<char*>(message.c_str());
    pubmsg.payloadlen = static_cast<int>(message.length());
    pubmsg.qos = topic_set->get_QOS();
    pubmsg.retained = 0;
    
    // Отправляем сообщение
    const int rc = MQTTAsync_sendMessage(
        this->client, 
        topic_set->get_path().c_str(),
        &pubmsg, 
        &opts);
    
    if (rc != MQTTASYNC_SUCCESS) 
    {
        // delete pubContext; // Освобождаем память при ошибке
        throw std::runtime_error(
            std::format("Failed to initiate publish to '{}', error: {}", 
                topic_set->get_path(), rc));
    }
}

void MainWindow::extract_topics()
{
    const std::string path_lesson =
        std::format("ntiurfu/{}/current_lesson", this->tag);
    const std::string path_temp_inside =
        std::format("ntiurfu/{}/temp/inside", this->tag);
    const std::string path_temp_outside =
        std::format("ntiurfu/{}/temp/outside", this->tag);
    const std::string path_time =
        std::format("ntiurfu/{}/time", this->tag);
    
    for(InSomnia::Topic &topic :
        this->callback_context.access_topics())
    {
        if(topic.get_path() == path_lesson)
        {
            this->topic_lesson = &topic;
        }
        else if(topic.get_path() == path_temp_inside)
        {
            this->topic_temp_inside = &topic;
        }
        else if(topic.get_path() == path_temp_outside)
        {
            this->topic_temp_outside = &topic;
        }
        else if(topic.get_path() == path_time)
        {
            this->topic_time = &topic;
        }
    }
    
    if(this->topic_lesson == nullptr)
    {
        throw std::runtime_error("topic_lesson is nullptr");
    }
    if(this->topic_temp_inside == nullptr)
    {
        throw std::runtime_error("topic_temp_inside is nullptr");
    }
    if(this->topic_temp_outside == nullptr)
    {
        throw std::runtime_error("topic_temp_outside is nullptr");
    }
    if(this->topic_time == nullptr)
    {
        throw std::runtime_error("topic_time is nullptr");
    }
}

void MainWindow::on_line_current_lesson_editingFinished()
{
    const QString current_line =
        ui->line_current_lesson->text();
    
    std::cout << "Setting current_line = "
        << current_line.toStdString() << "\n";
    std::cout.flush();
    
    const std::string current_line_str = current_line.toStdString();
    
    this->setting_parametr(this->topic_lesson, current_line_str);
    
}


void MainWindow::on_time_set_input_editingFinished()
{
    const QTime current_time =
        this->ui->time_set_input->time();
    
    if(!current_time.isValid())
    {
        this->ui->text_browser->append("Время задано некорректно\n");
        return;
    }
    
    const std::string current_time_str = 
        current_time.toString("hh:mm:ss").toStdString();
    
    std::cout << "Setting current_time = "
        << current_time_str << "\n";
    std::cout.flush();
    
    
    this->setting_parametr(this->topic_time, current_time_str);
}


void MainWindow::on_slider_temp_inside_sliderReleased()
{
    const int current_temp_inside =
        ui->slider_temp_inside->value();
    
    const QString current_temp_inside_qstr =
        QString::number(current_temp_inside);
    const std::string current_temp_inside_str = 
        current_temp_inside_qstr.toStdString();
    
    // this->ui->label_temp_inside->setText(current_temp_inside_qstr);
    
    std::cout << "Setting temp_inside = "
        << current_temp_inside_str << "\n";
    std::cout.flush();
    
    this->setting_parametr(
        this->topic_temp_inside, current_temp_inside_str);
}


void MainWindow::on_slider_temp_inside_valueChanged(int value)
{
    const int current_temp_inside =
        ui->slider_temp_inside->value();
    
    const QString current_temp_inside_qstr =
        QString::number(current_temp_inside);
    
    this->ui->label_temp_inside->setText(current_temp_inside_qstr);
}

void MainWindow::on_slider_temp_outside_sliderReleased()
{
    const int current_temp_outside =
        ui->slider_temp_outside->value();
    
    const QString current_temp_outside_qstr =
        QString::number(current_temp_outside);
    const std::string current_temp_outside_str = 
        current_temp_outside_qstr.toStdString();
    
    // this->ui->label_temp_inside->setText(current_temp_inside_qstr);
    
    std::cout << "Setting temp_outside = "
        << current_temp_outside_str << "\n";
    std::cout.flush();
    
    this->setting_parametr(
        this->topic_temp_outside, current_temp_outside_str);
}

void MainWindow::on_slider_temp_outside_valueChanged(int value)
{
    const int current_temp_outside =
        ui->slider_temp_outside->value();
    
    const QString current_temp_outside_qstr =
        QString::number(current_temp_outside);
    
    this->ui->label_temp_outside->setText(current_temp_outside_qstr);
}
