# Включаем поддержку CTest для управления тестами
include(CTest)

# Добавляем тесты 
add_executable(app_tests app_tests.cpp)

# Подключаем заголовочные файлы приложения
target_include_directories(app_tests
    PRIVATE
        ../include  
)

# Добавляем исходные файлы приложения для линковки с тестами
target_sources(app_tests
    PRIVATE
        ../src/console_app.cpp  
)

# Связываем с библиотекой 
target_link_libraries(app_tests
    PRIVATE
        library  
)

# Добавляем тест в CTest
add_test(NAME AppTests COMMAND app_tests)
