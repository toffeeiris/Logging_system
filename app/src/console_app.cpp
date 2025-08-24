#include "console_app.h"
#include <algorithm>
#include <limits>

// Конструктор: перемещаем логгер и сохраняем его тип
ConsoleApp::ConsoleApp(std::unique_ptr<Logger> logger)
    : logger(std::move(logger)), logger_type(this->logger->get_type())
{}

// Деструктор: закрываем приложение
ConsoleApp::~ConsoleApp() 
{
    close();
}

// Инициализация приложения
bool ConsoleApp::init()
{
    if (!logger)
    {
        std::cerr << "Логгер не инициализирован" << std::endl;
        return false;
    }

    run_flag = true;
    log_thread = std::thread(&ConsoleApp::log_tasks, this); // Запускаем фоновый поток
    return true;
}

// Фоновая задача для обработки логов из очереди
void ConsoleApp::log_tasks()
{
    Log task;
    while(run_flag || !log_queue.empty()) // Работаем пока приложение запущено или есть сообщения
    {
        if (log_queue.pop_with_wait(task)) // Блокирующее извлечение
        {
            // Отправляем сообщение через логгер
            LoggerError error = logger->log(task.msg, task.level);

            // Формируем запись для истории
            std::stringstream input;
            input << "[" << level_to_str(task.level) << "] " << task.msg;
            
            // Формируем запись для истории
            std::stringstream input;
            input << "[" << level_to_str(task.level) << "] " \
            << task.msg << " (" << status << ")";
            log_history.push_back(input.str());

            if (error != LoggerError::NONE)
                std::cerr << "Ошибка: " << static_cast<int>(error) << std::endl;
        }
    }

    // Обрабатываем оставшиеся сообщения после остановки
    while(log_queue.pop(task))
    {
        LoggerError error = logger->log(task.msg, task.level);
        if (error != LoggerError::NONE)
            std::cerr << "Ошибка: " << static_cast<int>(error) << std::endl;
    }
}                        

// Корректное закрытие приложения
void ConsoleApp::close()
 {
    run_flag = false;
    log_queue.stop(); // Останавливаем очередь
    
    if (log_thread.joinable()) 
        log_thread.join(); // Ждем завершения потока
}

// Основной цикл выполнения приложения
void ConsoleApp::run()
{
    std::cout << "=== Приложение ===" << std::endl;

    while (run_flag) 
    {
        std::cout << "\nТекущий уровень догирования: " << \
        level_to_str(logger->get_log_level()) << std::endl;
        show_menu();
        int choice = get_validated_input(1, 5, "Введите пункт меню: ");
        switch(choice)
        {
            case 1: add_log(); break;
            case 2: change_log_level(); break;
            case 3: show_history(); break;
            case 4: show_status(); break;
            case 5: run_flag = false; break;
            default: std::cout << "Нет такого пункта меню" << std::endl;
        }
    }
}

// Отображение меню пользователя
void ConsoleApp::show_menu()
{
    std::cout << "1 Добавить сообщение" << std::endl;
    std::cout << "2 Изменить уровень" << std::endl;
    std::cout << "3 Просмотреть историю" << std::endl;
    std::cout << "4 Статус логгера" << std::endl;
    std::cout << "5 Закрыть приложение" << std::endl;
    std::cout << "Допустимые уровни: DEBUG, INFO, ERROR\n" << std::endl;
}

// Добавление нового сообщения
void ConsoleApp::add_log() 
{
    LogLevel level = select_log_level();
    
    std::string input;
    std::cout << "Введите сообщение: ";
    getline(std::cin, input);

    if (validate_msg(input))
    {
        Log curr_log;
        curr_log.msg = input;
        curr_log.level = level;
        log_queue.push(curr_log); // Добавляем в очередь
        std::cout << "Сообщение добавлено в очередь" << std::endl;
    }
    else
        std::cout << "Ошибка: пустой ввод" << std::endl;
}

// Изменение уровня логирования
void ConsoleApp::change_log_level() 
{
    std::cout << "Текущий уровень: " << level_to_str(logger->get_log_level()) << std::endl;
    LogLevel new_level = select_log_level();
    logger->set_log_level(new_level);
}

// Отображение истории сообщений
void ConsoleApp::show_history()
{
    if (log_history.empty())
    {
        std::cout << "История пуста" << std::endl;
        return;
    }

    for (int i = 0; i < log_history.size(); ++i)
        std::cout << (i + 1) << " " << log_history[i] << std::endl;
}

// Отображение статуса приложения
void ConsoleApp::show_status() 
{
    std::cout << "Тип логгера: " << logger_type << std::endl;
    std::cout << "Текущий уровень: " << level_to_str(logger->get_log_level()) << std::endl;
    std::cout << "Ожидают отправки: " << log_queue.size() << std::endl;
    std::cout << "Всего сообщений: " << log_history.size() << std::endl;
}

// Выбор уровня логирования через меню
LogLevel ConsoleApp::select_log_level()
{
    std::cout << "Выберите уровень:" << std::endl;
    std::cout << "1 DEBUG (0 - низший уровень)" << std::endl;
    std::cout << "2 INFO (1)" << std::endl;
    std::cout << "3 ERROR (2)" << std::endl;

    int choice = get_validated_input(1, 3, "Введите пункт меню: ");

    switch(choice)
    {
        case 1: return LogLevel::DEBUG;
        case 2: return LogLevel::INFO;
        case 3: return LogLevel::ERROR;
        default: return logger->get_log_level();
    }
}

// Проверка числового ввода
int ConsoleApp::get_validated_input(int min, int max, const std::string& str)
{
    int value;
    while(true)
    {
        std::cout << str;
        if (std::cin >> value)
        {
            if (value >= min && value <= max)
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return value;
            }
        }

        std::cout << "Некорректный ввод" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// Валидация сообщения (не пустое и не только пробелы)
bool ConsoleApp::validate_msg(const std::string& msg)
{
    return !msg.empty() && msg.find_first_not_of(' ') != std::string::npos;
}

// Преобразование уровня логирования в строку
std::string ConsoleApp::level_to_str(LogLevel level)
{
    switch(level)
    {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Добавление тестового сообщения
void ConsoleApp::add_test_msg(const std::string& msg, LogLevel level)
{
    Log test_log{msg, level};
    log_queue.push(test_log);
}