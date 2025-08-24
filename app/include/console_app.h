#ifndef CONSOLE_APP_H
#define CONSOLE_APP_H

#include "logger.h"
#include "thread_queue.h"
#include <thread>

// Структура для хранения сообщения и его уровня
struct Log
{
    std::string msg;
    LogLevel level;
};

// Основной класс консольного приложения
class ConsoleApp
{
public:
    ConsoleApp(std::unique_ptr<Logger> logger); // Конструктор принимает уникальный указатель на логгер
    ~ConsoleApp();

    ConsoleApp(const ConsoleApp&) = delete; // Запрещаем копирование
    ConsoleApp& operator=(const ConsoleApp&) = delete;

    bool init(); // Инициализация приложения
    void run();  // Основной цикл выполнения
    void close(); // Завершение работы

    // Методы для тестирования
    void add_test_msg(const std::string& msg, LogLevel level);
    size_t get_history() const {return log_history.size();} // Получение размера истории
    size_t get_queue_size() const {return log_queue.size();} // Получение размера очереди

private:
    void log_tasks(); // Фоновая задача для обработки логов

    // Методы пользовательского интерфейса
    void show_menu(); // Отображение меню
    void add_log();   // Добавление лога
    void change_log_level(); // Изменение уровня логирования
    void show_history(); // Показать историю
    void show_status();  // Показать статус

    LogLevel select_log_level(); // Выбор уровня логирования
    bool validate_log_level(const std::string& level_str); // Валидация уровня
    bool validate_msg(const std::string& message); // Валидация сообщения
    int get_validated_input(int min, int max, const std::string& prompt); // Получение валидного ввода
    
    static std::string level_to_str(LogLevel level); // Преобразование уровня в строку

    // Члены класса
    std::unique_ptr<Logger> logger; // Указатель на логгер
    ThreadQueue<Log> log_queue;     // Потокобезопасная очередь сообщений
    std::vector<std::string> log_history; // История сообщений
    std::atomic<bool> run_flag = false; // Флаг работы приложения (атомарный для потокобезопасности)
    std::thread log_thread;        // Поток для обработки сообщений
    std::string logger_type;        // Тип логгера (для отображения)
};

#endif // CONSOLE_APP_H