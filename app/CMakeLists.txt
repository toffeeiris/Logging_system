# Создаем исполняемый файл
add_executable(console_app
    src/main.cpp
    src/console_app.cpp
)

# Подключаем заголовочные файлы приложения
target_include_directories(console_app
    PRIVATE
<<<<<<< HEAD
        ${CMAKE_CURRENT_SOURCE_DIR}/include  
        ${CMAKE_SOURCE_DIR}/library/include  
=======
        /include
>>>>>>> 2b9e25a48328663f1635c68e6a0c22880b87fa7c
)

# Связываем приложение с библиотекой
target_link_libraries(console_app
    PRIVATE
        library
)

# Установка исполняемого файла в директорию bin
install(TARGETS console_app DESTINATION bin)

# Тесты для приложения
add_subdirectory(tests)
