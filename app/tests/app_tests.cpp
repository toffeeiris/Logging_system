#include "console_app.h"
#include "logger.h"
#include "file_logger.h" 
#include <filesystem>
#include <vector>
#include <atomic>

namespace fs = std::filesystem;

void print(const std::string& name, bool result)
{
    std::cout << "Тест: " << name;
    std::cout << (result ? " +" : " -") << std::endl;
}

void clean()
{
    std::vector<std::string> files = 
    {
        "test_init.log",
        "test_level.log",
        "test_queue.log",
        "test_thread.log",
        "test_history.log",
        "test_input.log",
        "test_close.log"
    };   

    for (const auto& file : files) 
        if (fs::exists(file)) fs::remove(file);
}

// Тест инициализации приложения на примере FileLogger
bool test_app_init()
{
    auto logger = create_file_logger("test_init.log", LogLevel::INFO);
    ConsoleApp app(std::move(logger));
    return app.init();
}

// Тест фильтрации по уровню
bool test_app_level()
{
    auto logger = create_file_logger("test_level.log", LogLevel::ERROR);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;
    
    app.add_test_msg("debug msg", LogLevel::DEBUG);
    app.add_test_msg("info msg", LogLevel::INFO);
    app.add_test_msg("error msg", LogLevel::ERROR);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    app.close();
    
    // Проверка что в файле только ERROR сообщения
    std::ifstream file("test_level.log");
    std::string line;
    bool has_debug = false, has_info = false, has_error = false;
    
    while (std::getline(file, line)) 
    {
        if (line.find("[DEBUG]") != std::string::npos) has_debug = true;
        if (line.find("[INFO]") != std::string::npos) has_info = true;
        if (line.find("[ERROR]") != std::string::npos) has_error = true;
    }
    
    return !has_debug && !has_info && has_error;
}

// Тест работы очереди
bool test_msg_queue()
{
    auto logger = create_file_logger("test_queue.log", LogLevel::INFO);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;

    const int msg_cnt = 5;
    for (int i = 0; i < msg_cnt; ++i) 
        app.add_test_msg("test msg " + std::to_string(i), LogLevel::INFO);
    
    if (app.get_queue_size() != msg_cnt) return false;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    app.close();

    std::ifstream file("test_queue.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) lines++;
    
    return lines == msg_cnt;
}

// Тест многопоточности
bool test_app_thread()
{
    auto logger = create_file_logger("test_thread.log", LogLevel::DEBUG);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;

    std::vector<std::thread> threads;
    const int thread_cnt = 3;
    const int msg_cnt = 10;

    for (int i = 0; i < thread_cnt; ++i)
    {
        threads.emplace_back([&app, i]() 
        {
            for (int j = 0; j < msg_cnt; ++j) 
            {
                app.add_test_msg("thread " + std::to_string(i) + " msg " + std::to_string(j), 
                                  LogLevel::INFO);
            }
        });
    }

    for (auto& t : threads) 
        t.join();

    // Ждем пока все сообщения обработаются
    int wait_count = 0;
    while (app.get_queue_size() > 0 && wait_count < 50)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wait_count++;
    }

    app.close();
    
    std::ifstream file("test_thread.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) 
        lines++;
    
    return lines == thread_cnt * msg_cnt;
}

// Тест истории сообщений
bool test_app_history()
{
    auto logger = create_file_logger("test_history.log", LogLevel::INFO);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;

    const int test_msg = 3;
    for (int i = 0; i < test_msg; ++i) 
        app.add_test_msg("history test " + std::to_string(i), LogLevel::INFO);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool is_history = app.get_history() == test_msg;
    app.close();

    std::ifstream file("test_history.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) 
        lines++;
    
    return is_history && lines == test_msg;
}

// Тест обработки невалидного ввода
bool test_app_invalid_input()
{
    auto logger = create_file_logger("test_input.log", LogLevel::INFO);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;

    app.add_test_msg("", LogLevel::INFO);
    app.add_test_msg("   ", LogLevel::INFO);
    app.add_test_msg("normal msg", LogLevel::INFO);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    app.close();
    
    // Проверяем что только валидные сообщения записаны
    std::ifstream file("test_input.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) 
        if (line.find("normal msg") != std::string::npos) lines++;
    
    return lines == 1;
}

// Тест корректного закрытия
bool test_app_close()
{
    auto logger = create_file_logger("test_close.log", LogLevel::INFO);
    ConsoleApp app(std::move(logger));
    
    if (!app.init()) return false;
    const int msg_cnt = 3;
    
    for (int i = 0; i < msg_cnt; ++i) 
        app.add_test_msg("close test " + std::to_string(i), LogLevel::INFO);
    
    if (app.get_queue_size() != msg_cnt) return false;
    app.close();
    
    std::ifstream file("test_close.log");
    int lines = 0;
    std::string line;
    while (std::getline(file, line)) 
        lines++;
    
    return lines == msg_cnt;
}

int main()
{
    std::cout << "Тесты ConsoleApp: " << std::endl;
    
    print("Инициализация", test_app_init());
    print("Фильтрация по уровню", test_app_level());
    print("Очередь сообщений", test_msg_queue());
    print("Потокобезопасность", test_app_thread());
    print("История сообщений", test_app_history());
    print("Обработка ошибок", test_app_invalid_input());
    print("Корректное закрытие", test_app_close());

    clean();
    return 0;
}