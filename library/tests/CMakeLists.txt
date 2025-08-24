# Включаем поддержку CTest для управления тестами
include(CTest)

# Добавляем тесты
add_executable(logger_tests logger_tests.cpp)

# Подключаем заголовочные файлы библиотеки
target_include_directories(logger_tests
    PRIVATE
        ../include
)

# Связываем с библиотекой
target_link_libraries(logger_tests
    PRIVATE
        library
)

# Добавляем тест в CTest
add_test(NAME LoggerTests COMMAND logger_tests)
