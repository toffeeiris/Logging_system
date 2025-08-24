#include "logger.h"
#include "file_logger.h"
#include "socket_logger.h"
#include <filesystem>
#include <thread>
#include <vector>

namespace fs = std::filesystem; 

// Функция для вывода результатов тестов
void print(const std::string& name, bool result)
{
    std::cout << "Тест: " << name;
    std::cout << (result ? " +" : " -") << std::endl; // "+" для успеха, "-" для неудачи
}

// Функция очистки тестовых файлов после выполнения тестов
void clean()
{
    std::vector<std::string> files = 
    {
        "test_create.log",
        "test_level.log",
        "test_format.log",
        "test_multithreaded.log"
    };   

    // Удаляем каждый тестовый файл, если он существует
    for (const auto& file : files) 
        if (fs::exists(file)) fs::remove(file);
}

// FileLogger tests 

// Тест: Создание файлового логгера и запись в файл
bool test_file_create()
{
    // Создаем логгер с уровнем INFO
    auto logger = create_file_logger("test_create.log", LogLevel::INFO);
    // Проверяем, что логгер создан и запись прошла успешно
    return logger && logger->log("test", LogLevel::INFO) == LoggerError::NONE;
}

// Тест: Попытка записи в несуществующий путь
bool test_file_invalid_path() 
{
    // Пытаемся создать логгер с невалидным путем
    auto logger = create_file_logger("/invalid/path/test.log", LogLevel::INFO);
    // Должна вернуться ошибка FILE_OPEN_FAILED
    return logger && logger->log("test", LogLevel::INFO) == LoggerError::FILE_OPEN_FAILED;
}

// Тест: Проверка фильтрации по уровню логирования
bool test_file_level()
{
    auto logger = create_file_logger("test_level.log", LogLevel::INFO);

    // Отправляем сообщения разных уровней
    logger->log("debug", LogLevel::DEBUG);   // Должно быть отфильтровано (уровень DEBUG < INFO)
    logger->log("info", LogLevel::INFO);     // Должно быть записано
    logger->log("error", LogLevel::ERROR);   // Должно быть записано

    // Читаем файл и проверяем содержимое
    std::ifstream file("test_level.log");
    std::string line;
    bool has_debug = false, has_info = false, has_error = false;
    
    while (std::getline(file, line)) 
    {
        if (line.find("debug") != std::string::npos) has_debug = true;
        if (line.find("info") != std::string::npos) has_info = true;
        if (line.find("error") != std::string::npos) has_error = true;
    }
    
    // DEBUG не должно быть, INFO и ERROR должны быть
    return !has_debug && has_info && has_error;
}

// Тест: Проверка формата сообщения
bool test_file_format()
{
    auto logger = create_file_logger("test_format.log", LogLevel::INFO);
    logger->log("test msg", LogLevel::INFO);

    // Читаем первую строку из файла
    std::ifstream file("test_format.log");
    std::string line;
    getline(file, line);

    // Проверяем, что строка содержит уровень INFO и тестовое сообщение
    return line.find("[INFO]") != std::string::npos &&
           line.find("test msg") != std::string::npos;
}

// Тест: Многопоточное тестирование
bool test_file_multithreaded()
{
    // Создаем логгер с уровнем DEBUG (принимает все сообщения)
    auto logger = create_file_logger("test_multithreaded.log", LogLevel::DEBUG);
    std::vector<std::thread> threads;
    const int thread_cnt = 3;    
    const int msg_cnt = 10;   

    // Создаем и запускаем потоки
    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back([&logger, i]() 
        {
            for (int j = 0; j < msg_cnt; ++j) 
            {
                // Каждый поток пишет свои сообщения
                logger->log("thread " + std::to_string(i) + " msg " + std::to_string(j), LogLevel::INFO);
            }
        });
    }

    // Ждем завершения всех потоков
    for (auto& t : threads) 
        t.join();
    logger.reset(); // Освобождаем логгер 

    // Подсчитываем количество строк в файле
    std::ifstream file("test_multithreaded.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line))
        lines++;

    return lines == thread_cnt * msg_cnt;
}

// SocketLogger tests 

// Тест: Создание объекта SocketLogger (без реального подключения)
bool test_socket_create()
{
    // Просто создаем объект - без попытки подключения
    SocketLogger logger("127.0.0.1", 8080, LogLevel::INFO);
    // Проверяем, что тип и уровень установлены правильно
    return logger.get_type() == "socket" && 
           logger.get_log_level() == LogLevel::INFO;
}

// Тест: Фильтрация по уровню в SocketLogger
bool test_socket_level()
{
    SocketLogger logger("127.0.0.1", 8080, LogLevel::INFO);
    
    // DEBUG сообщение должно быть отфильтровано 
    LoggerError result = logger.log("debug", LogLevel::DEBUG);
    return result == LoggerError::NONE; // Должен вернуть NONE - сообщение отфильтровано
}

// Тест: Создание логгера с неверными параметрами подключения
bool test_socket_invalid_connection()
{
    // Пытаемся создать логгер с несуществующим хостом
    auto logger = create_socket_logger("invalid.host", 8080, LogLevel::INFO);
    // Должен вернуть nullptr, так как подключение не удалось
    return logger == nullptr;
}

// Главная функция тестирования
int main()
{
    std::cout << "Тесты FileLogger: " << std::endl;
    print("Создание", test_file_create());
    print("Некорректный путь", test_file_invalid_path());
    print("Фильтрация по уровню", test_file_level());
    print("Формат сообщения", test_file_format());
    print("Многопоточность", test_file_multithreaded());

    std::cout << "\nТесты SocketLogger: " << std::endl;
    print("Создание объекта", test_socket_create());
    print("Фильтрация по уровню", test_socket_level());
    print("Неверное подключение", test_socket_invalid_connection());

    clean(); // Очищаем тестовые файлы
    return 0;
}