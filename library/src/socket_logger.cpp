#include "socket_logger.h"

// Конструктор сокетного логгера
SocketLogger::SocketLogger(const std::string& host, int port, LogLevel level)
    : host(host), port(port), sockfd(-1), log_level(level), init_flag(false)
{}

// Деструктор 
SocketLogger::~SocketLogger()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    close_socket(); // Закрытие соединения
}

// Подключение к серверу
LoggerError SocketLogger::connect_to_server()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Создание TCP сокета
    if (sockfd == -1)
    {
        std::cerr << "Ошибка создания" << std::endl;
        return LoggerError::FILE_OPEN_FAILED;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;          // IPv4
    server_addr.sin_port = htons(port);        // Порт в сетевом порядке байт
    
    // Преобразование адреса из текстового в бинарный формат
    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0)
    {
        std::cerr << "Некорретный адрес: " << host << std::endl;
        close_socket();
        return LoggerError::FILE_OPEN_FAILED;
    }

    // Установка соединения с сервером
    if (connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Не удалось подключится к " << host << ":" << port << std::endl;
        close_socket();
        return LoggerError::FILE_OPEN_FAILED;
    }

    std::cout << "Подключение к серверу по " << host << ":" << port << std::endl;
    return LoggerError::NONE;
}

// Закрытие сокета
void SocketLogger::close_socket()
{
    if (sockfd != -1) 
    {
        close(sockfd);
        sockfd = -1; // Сброс дескриптора
    }
}

// Инициализация соединения
LoggerError SocketLogger::init()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    if (init_flag) 
        return LoggerError::NONE; // Уже инициализирован
    
    LoggerError result = connect_to_server();
    if (result == LoggerError::NONE) 
        init_flag = true; // Установка флага успешной инициализации
    
    return result;
}

// Повторное подключение при разрыве соединения
LoggerError SocketLogger::reconnect()
{
    std::lock_guard<std::mutex> lock(log_mutex);
    close_socket();
    return connect_to_server(); // Попытка переподключения
}

// Метод логирования через сокет
LoggerError SocketLogger::log(const std::string& msg, LogLevel level)
{
    if (level < log_level) return LoggerError::NONE; // Фильтрация по уровню

    if (!init_flag) 
    {
        std::cerr << "Логгер сокетов не инициализирован" << std::endl;
        return LoggerError::FILE_OPEN_FAILED;
    }

    std::string curr_msg = msg_format(level, msg) + "\n"; // Форматирование сообщения
    std::lock_guard<std::mutex> lock(log_mutex);
    
    // Проверка соединения и переподключение при необходимости
    if (sockfd == -1) 
    {
        LoggerError result = reconnect();
        if (result != LoggerError::NONE) 
            return result;
    }

    // Отправка сообщения через сокет
    ssize_t bytes_sent = send(sockfd, curr_msg.c_str(), curr_msg.size(), 0);
    if (bytes_sent == -1) 
    {
        std::cerr << "Не удалось отправить сообщение" << std::endl;
        close_socket();
        return LoggerError::WRITE_FAILED;
    }

    return LoggerError::NONE;
}

// Фабричный метод для создания сокетного логгера
std::unique_ptr<Logger> create_socket_logger(const std::string& host, int port, LogLevel level)
{
    auto logger = std::make_unique<SocketLogger>(host, port, level);
    LoggerError result = logger->init(); // Инициализация соединения
    if (result != LoggerError::NONE) 
        return nullptr; // Возврат nullptr при ошибке инициализации
    
    return logger;
}