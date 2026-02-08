#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <signal.h>
// #include <pthread.h>
// #include "MQTTAsync.h"

// #define ADDRESS     "test.mosquitto.org"
// #define CLIENTID    "somni_soni"
// #define TOPIC       "ntiurfu/somni_soni/temp/inside"
// #define QOS         1
// #define TIMEOUT     10000L

// volatile int connected = 0;
// volatile int subscribed = 0;
// volatile int finished = 0;

// // Callback при успешной подписке
// void onSubscribe(void* context, MQTTAsync_successData* response) {
//     printf("Успешно подписались на топик: %s\n", TOPIC);
//     subscribed = 1;
// }

// // Callback при неудачной подписке
// void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
//     printf("Не удалось подписаться на топик, код: %d\n", 
//            response ? response->code : 0);
//     finished = 1;
// }

// // Callback при успешном подключении
// void onConnect(void* context, MQTTAsync_successData* response) {
//     printf("Успешное подключение к брокеру\n");
//     connected = 1;
    
//     MQTTAsync client = (MQTTAsync)context;
//     MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
//     int rc;
    
//     // Подписка на топик
//     opts.onSuccess = onSubscribe;
//     opts.onFailure = onSubscribeFailure;
//     opts.context = client;
    
//     rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts);
//     if (rc != MQTTASYNC_SUCCESS) {
//         printf("Ошибка при попытке подписки: %d\n", rc);
//         finished = 1;
//     }
// }

// // Callback при неудачном подключении
// void onConnectFailure(void* context, MQTTAsync_failureData* response) {
//     printf("Не удалось подключиться к брокеру, код: %d\n", 
//            response ? response->code : 0);
//     finished = 1;
// }

// // Callback при получении сообщения
// int onMessage(void* context, char* topicName, int topicLen, 
//               MQTTAsync_message* message) {
//     printf("Асинхронно получено сообщение:\n");
//     printf("  Топик: %s\n", topicName);
//     printf("  Сообщение: %.*s\n", message->payloadlen, (char*)message->payload);
//     printf("  QoS: %d\n", message->qos);
    
//     MQTTAsync_freeMessage(&message);
//     MQTTAsync_free(topicName);
//     return 1;
// }

// // Callback при потере соединения
// void onConnectionLost(void* context, char* cause) {
//     printf("Соединение потеряно\n");
//     if (cause)
//         printf("Причина: %s\n", cause);
    
//     connected = 0;
//     subscribed = 0;
// }

// // Callback при отправке сообщения
// void onDeliveryComplete(void* context, MQTTAsync_token token) {
//     printf("Сообщение с токеном %d доставлено\n", token);
// }

// // Callback при отключении
// void onDisconnect(void* context, MQTTAsync_successData* response) {
//     printf("Успешно отключились от брокера\n");
//     finished = 1;
// }

// // Обработчик сигналов
// void signal_handler(int sig) {
//     printf("\nПолучен сигнал %d, завершаем работу...\n", sig);
//     finished = 1;
// }

// int main(int argc, char* argv[]) {
//     MQTTAsync client;
//     MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
//     MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
//     int rc;
    
//     // Настройка обработчика сигналов
//     signal(SIGINT, signal_handler);
//     signal(SIGTERM, signal_handler);
    
//     // Создание асинхронного клиента
//     rc = MQTTAsync_create(&client, ADDRESS, CLIENTID,
//                          MQTTCLIENT_PERSISTENCE_NONE, NULL);
//     if (rc != MQTTASYNC_SUCCESS) {
//         printf("Ошибка создания клиента: %d\n", rc);
//         return EXIT_FAILURE;
//     }
    
//     // Установка callback-функций
//     rc = MQTTAsync_setCallbacks(client, client, onConnectionLost, 
//                                onMessage, onDeliveryComplete);
//     if (rc != MQTTASYNC_SUCCESS) {
//         printf("Ошибка установки callback-ов: %d\n", rc);
//         MQTTAsync_destroy(&client);
//         return EXIT_FAILURE;
//     }
    
//     // Настройка параметров подключения
//     conn_opts.keepAliveInterval = 20;
//     conn_opts.cleansession = 1;
//     conn_opts.onSuccess = onConnect;
//     conn_opts.onFailure = onConnectFailure;
//     conn_opts.context = client;
    
//     // Асинхронное подключение к брокеру
//     printf("Подключаемся к брокеру %s...\n", ADDRESS);
//     rc = MQTTAsync_connect(client, &conn_opts);
//     if (rc != MQTTASYNC_SUCCESS) {
//         printf("Ошибка инициализации подключения: %d\n", rc);
//         MQTTAsync_destroy(&client);
//         return EXIT_FAILURE;
//     }
    
//     // Основной цикл - не блокирует поток
//     printf("Основной поток продолжает работу...\n");
//     while (!finished) {
//         // Здесь может выполняться другая полезная работа
//         printf(".");
//         fflush(stdout);
        
//         // Проверяем состояние подключения
//         if (connected && subscribed) {
//             // Можем выполнять какие-то действия
//             // Например, публиковать сообщения
//         }
        
//         sleep(1); // Небольшая пауза
//     }
    
//     // Отключение от брокера
//     if (connected) {
//         disc_opts.onSuccess = onDisconnect;
//         disc_opts.context = client;
        
//         rc = MQTTAsync_disconnect(client, &disc_opts);
//         if (rc != MQTTASYNC_SUCCESS) {
//             printf("Ошибка инициализации отключения: %d\n", rc);
//         }
        
//         // Даем время на завершение отключения
//         sleep(1);
//     }
    
//     // Очистка ресурсов
//     MQTTAsync_destroy(&client);
    
//     printf("\nПрограмма завершена\n");
//     return EXIT_SUCCESS;
// }
