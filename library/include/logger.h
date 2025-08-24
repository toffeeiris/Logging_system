#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <sstream>
#include <fstream>

// Уровни логирования
enum class LogLevel
{
    DEBUG,   // Отладочная информация
    INFO,    // Информационные сообщения
    ERROR    // Сообщения об ошибках
};

// Коды ошибок логгера
enum class LoggerError
{
    NONE,            // Ошибок нет
    FILE_OPEN_FAILED, // Не удалось открыть файл
    WRITE_FAILED      // Ошибка записи
};

// Базовый абстрактный класс логгера
class Logger
{
public:
    virtual ~Logger() = default; 
    
    // Чисто виртуальные методы
    virtual LoggerError log(const std::string& msg, LogLevel level) = 0;
    virtual void set_log_level(LogLevel level) = 0;
    virtual LogLevel get_log_level() const = 0;
    virtual std::string get_type() const = 0;

    // Вспомогательные методы для логирования
    void debug(const std::string& msg)
    {
        log(msg, LogLevel::DEBUG);
    }

    void info(const std::string& msg)
    {
        log(msg, LogLevel::INFO);
    }

    void error(const std::string& msg)
    {
        log(msg, LogLevel::ERROR);
    }

protected:
    // Статический метод для форматирования сообщения
    static std::string msg_format(LogLevel level, const std::string& msg)
    {
        auto curr = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(curr);

        std::stringstream ss;
        // Формат: [2024-01-15 14:30:25] [INFO] Сообщение
        ss << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %X") << "] "\
        << "[" << level_to_str(level) << "] " << msg;

        return ss.str();
    }

private:
    // Преобразование уровня логирования в строку
    static std::string level_to_str(LogLevel level)
    {
        switch(level)
        {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Фабричные функции для создания логгеров
std::unique_ptr<Logger> create_file_logger(const std::string& file_name, LogLevel level = LogLevel::INFO);
std::unique_ptr<Logger> create_socket_logger(const std::string& host, int port, LogLevel level = LogLevel::INFO);

#endif // LOGGER_H