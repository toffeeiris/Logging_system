#ifndef SOCKET_LOGGER_H
#define SOCKET_LOGGER_H

#include "logger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Класс сетевого логгера, наследуется от базового Logger
class SocketLogger : public Logger
{
public:
    SocketLogger(const std::string& host, int port, LogLevel level = LogLevel::INFO);
    ~SocketLogger();

    // Запрещаем копирование
    SocketLogger(const SocketLogger&) = delete;
    SocketLogger& operator=(const SocketLogger&) = delete;

    // Реализация виртуальных методов
    LoggerError log(const std::string& msg, LogLevel level) override;
    std::string get_type() const override { return "socket"; }
    
    // Инициализация соединения
    LoggerError init();

    // Установка и получение уровня логирования
    void set_log_level(LogLevel level) override
    {
        log_level = level;
    }

    LogLevel get_log_level() const override
    {
        return log_level;
    }

    // Проверка состояния
    bool is_init() const { return init_flag; }          // Проверка инициализации
    bool is_connected() const { return sockfd != -1; }  // Проверка соединения
    
    // Повторное соединение
    LoggerError reconnect();

private:
    std::string host;     // Хост для подключения
    int port;             // Порт для подключения
    int sockfd;           // Дескриптор сокета
    LogLevel log_level;  // Текущий уровень логирования
    std::mutex log_mutex; // Мьютекс для потокобезопасности
    bool init_flag;       // Флаг инициализации

    // Внутренние методы
    LoggerError connect_to_server(); // Подключение к серверу
    void close_socket();             // Закрытие сокета
};

#endif // SOCKET_LOGGER_H