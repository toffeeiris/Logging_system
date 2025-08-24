# Создаем библиотеку
add_library(library
    src/file_logger.cpp
    src/socket_logger.cpp
)

target_include_directories(library
    PUBLIC 
        include/      
    PRIVATE
        src/          
)

# Устанавливаем свойства библиотеки
set_target_properties(library PROPERTIES
    VERSION 1.0.0  # Номер версии библиотеки
    SOVERSION 1    # Номер версии ABI (для совместимости)
)

# Установка библиотеки и заголовочных файлов
install(TARGETS library
    LIBRARY DESTINATION lib   # Динамические библиотеки (.so, .dll)
    ARCHIVE DESTINATION lib   # Статические библиотеки (.a, .lib)
    RUNTIME DESTINATION bin   # Исполняемые файлы 
)

# Устанавливаем заголовочные файлы для использования другими проектами
install(DIRECTORY include/ DESTINATION include)

# Тесты для библиотеки 
add_subdirectory(tests)
