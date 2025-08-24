#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include "logger.h"

// Класс файлового логгера, наследуется от базового Logger
class FileLogger : public Logger
{
public:
    FileLogger(const std::string& file_name, LogLevel level = LogLevel::INFO);
    ~FileLogger();

    // Запрещаем копирование
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    // Реализация виртуальных методов
    LoggerError log(const std::string& msg, LogLevel level) override;
    std::string get_type() const override { return "file"; }

    // Установка и получение уровня логирования
    void set_log_level(LogLevel level) override
    {
        log_level = level;
    }

    LogLevel get_log_level() const override
    {
        return log_level;
    }

private:
    std::string name;           // Имя файла
    std::ofstream log_file;     // Файловый поток для записи
    LogLevel log_level;        // Текущий уровень логирования
    std::mutex log_mutex;       // Мьютекс для потокобезопасности

};

#endif // FILE_LOGGER_H