#include "console_app.h"
#include "logger.h"
#include "file_logger.h" 
#include "socket_logger.h" 

// Парсинг строки в уровень логирования
LogLevel parse_log_level(const std::string& level_str) 
{
    if (level_str == "DEBUG" || level_str == "debug") return LogLevel::DEBUG;
    if (level_str == "INFO" || level_str == "info") return LogLevel::INFO;
    if (level_str == "ERROR" || level_str == "error") return LogLevel::ERROR;
    return LogLevel::INFO; // По умолчанию
}

// Вывод правил использования
void print_rules()
{
    std::cerr << "Некорректный ввод" << std::endl;
    std::cout << "Формы ввода: " << std::endl;
    std::cout << "  file <filename> [level]      - File logger" << std::endl;
    std::cout << "  socket <host> <port> [level] - Socket logger" << std::endl;
    std::cout << "Уровни: DEBUG, INFO, ERROR (по умолчанию: INFO)" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        print_rules();
        return 1;
    }

    std::string type = argv[1];
    std::unique_ptr<Logger> logger;
    LogLevel level = LogLevel::INFO;

    // Обработка file logger
    if (type == "file")
    {
        if (argc < 3)
        {
            std::cerr << "Ошибка: указаны не все параметры" << std::endl;
            print_rules();
            return 1;
        }
        
        std::string filename = argv[2];

        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") 
        {
            std::cerr << "Ошибка: имя файла должно иметь расширение .txt" << std::endl;
            return 1;
        }
        
        if (argc >= 4) 
            level = parse_log_level(argv[3]);
        
        logger = create_file_logger(filename, level);
        if (!logger)
        {
            std::cerr << "Ошибка: не удалось создать логгер " << filename << std::endl;
            return 1;
        }
    }
    // Обработка socket logger
    else if (type == "socket")
    {
        if (argc < 4)
        {
            std::cerr << "Ошибка: указаны не все параметры" << std::endl;
            print_rules();
            return 1;
        }
        
        std::string host = argv[2];
        int port = std::atoi(argv[3]);
        if (port <= 0 || port > 65535) 
        {
            std::cerr << "Ошибка: некорректный порт" << std::endl;
            return 1;
        }
        
        if (argc >= 5) 
            level = parse_log_level(argv[4]);
        
        auto socket_logger = std::make_unique<SocketLogger>(host, port, level);
        LoggerError init_result = socket_logger->init();
        
        if (init_result != LoggerError::NONE) 
        {
            std::cerr << "Ошибка: не удалось создать логгер (" 
                      << static_cast<int>(init_result) << ")" << std::endl;
            return 1;
        }
        
        logger = std::move(socket_logger);
    }
    else
    {
        std::cerr << "Ошибка: неизвестный тип логгера" << std::endl;
        print_rules();
        return 1;
    }

    if (!logger)
    {
        std::cerr << "Ошибка: не удалось создать логгер" << std::endl;
        return 1;
    }

    // Создание и запуск приложения
    ConsoleApp app(std::move(logger));
    if (!app.init())
    {
        std::cerr << "Ошибка: не удалось создать приложение" << std::endl;
        return 1;
    }

    app.run();
    app.close();
    return 0;
}