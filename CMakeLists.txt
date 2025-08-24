# Указываем минимальную требуемую версию CMake
cmake_minimum_required(VERSION 3.10)

# Задаем имя проекта
project(logging_system)

# Настройка стандарта C++ - используем C++17
set(CMAKE_CXX_STANDARD 17)
# Требуем обязательного соблюдения стандарта
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Опция для выбора типа библиотеки (статическая/динамическая)
# По умолчанию собираем динамические библиотеки (ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Добавляем поддиректории с исходным кодом
add_subdirectory(library)  # Директория с библиотекой логирования
add_subdirectory(app)      # Директория с приложением
