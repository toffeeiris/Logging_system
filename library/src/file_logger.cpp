#include "file_logger.h"

// Конструктор файлового логгера
FileLogger::FileLogger(const std::string& file_name, LogLevel level)
    : name(file_name), log_level(level)
{}

// Деструктор 
FileLogger::~FileLogger()
{
    std::lock_guard<std::mutex> lock(log_mutex); // Защита от гонки данных
    if (log_file.is_open())
        log_file.close();    // Закрытие файла при уничтожении объекта
}

// Основной метод логирования
LoggerError FileLogger::log(const std::string& msg, LogLevel level)
{
    if (level < log_level) return LoggerError::NONE; // Пропуск сообщений ниже установленного уровня

    std::lock_guard<std::mutex> lock(log_mutex); // Потокобезопасность
    if (!log_file.is_open()) 
    {
        // Открытие файла в режиме добавления 
        log_file.open(name, std::ios::out | std::ios::app);
        if (!log_file.is_open()) 
            return LoggerError::FILE_OPEN_FAILED; // Ошибка открытия файла
    }

    // Форматирование и запись сообщения
    log_file << msg_format(level, msg) << std::endl;

    if (log_file.fail())
        return LoggerError::WRITE_FAILED; // Ошибка записи

    log_file.flush(); // Принудительная запись в файл
    return LoggerError::NONE; // Успешное выполнение
}

// Фабричный метод для создания файлового логгера
std::unique_ptr<Logger> create_file_logger(const std::string& file_name, LogLevel level)
{
    return  std::make_unique<FileLogger>(file_name, level);
}