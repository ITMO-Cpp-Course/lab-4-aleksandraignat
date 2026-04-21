#include <catch2/catch_all.hpp>

#include "resource_core/file_handle.hpp"
#include "resource_core/resource_manager.hpp"

#include <fstream>
#include <thread>
#include <vector>

using namespace lab4::resource;

// ====================== Вспомогательные функции ======================

/**
 * @brief Создаёт временный файл с заданным содержимым
 * @param filename Имя файла
 * @param content Содержимое файла
 */
static void create_test_file(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    file << content;
    file.close();
}

/**
 * @brief Удаляет временный файл
 * @param filename Имя файла
 */
static void delete_test_file(const std::string& filename)
{
    std::remove(filename.c_str());
}

// ====================== Тесты для FileHandle ======================

/**
 * @brief Тест проверяет RAII поведение FileHandle:
 *        - файл автоматически открывается в конструкторе
 *        - файл автоматически закрывается в деструкторе
 *        - данные успешно записываются и читаются
 */
TEST_CASE("FileHandle открывает и автоматически закрывает файл", "[file_handle]")
{
    const std::string test_file = "test_raii.txt";

    // Блок: создаём хендл, пишем данные, при выходе из блока файл закроется
    {
        FileHandle fh(test_file, "w");
        REQUIRE(fh.is_open());
        fh.write_line("Hello RAII");
    }

    // Проверяем, что данные действительно записались
    FileHandle fh2(test_file, "r");
    REQUIRE(fh2.is_open());
    auto line = fh2.read_line();
    REQUIRE(line == "Hello RAII");

    delete_test_file(test_file);
}

/**
 * @brief Тест проверяет обработку ошибок:
 *        - открытие несуществующего файла
 *        - чтение из закрытого файла
 *        - запись в закрытый файл
 *        - повторное открытие уже открытого файла
 */
TEST_CASE("FileHandle бросает исключение при ошибках", "[file_handle]")
{
    const std::string test_file = "test_error.txt";

    // Попытка открыть несуществующий файл должна бросить исключение
    REQUIRE_THROWS_AS(FileHandle("/nonexistent/file.txt", "r"), ResourceError);

    FileHandle fh(test_file, "w");
    fh.close();  // Закрываем файл

    // Все операции с закрытым файлом должны бросать исключение
    REQUIRE_THROWS_AS(fh.read_line(), ResourceError);
    REQUIRE_THROWS_AS(fh.write_line("test"), ResourceError);
    REQUIRE_THROWS_AS(fh.open(test_file, "r"), ResourceError);

    delete_test_file(test_file);
}

/**
 * @brief Тест проверяет семантику перемещения:
 *        - при перемещении владение ресурсом передаётся новому объекту
 *        - исходный объект становится невалидным
 */
TEST_CASE("FileHandle поддерживает перемещение", "[file_handle]")
{
    const std::string test_file = "test_move.txt";

    FileHandle fh1(test_file, "w");
    fh1.write_line("Move test");

    // Перемещаем владение из fh1 в fh2
    FileHandle fh2 = std::move(fh1);

    // fh1 больше не владеет ресурсом
    REQUIRE_FALSE(fh1.is_open());
    // fh2 теперь владеет
    REQUIRE(fh2.is_open());

    delete_test_file(test_file);
}

// ====================== Тесты для ResourceManager ======================

/**
 * @brief Тест проверяет кеширование:
 *        - повторный запрос того же файла возвращает тот же shared_ptr
 *        - счётчик ссылок корректно увеличивается
 */
TEST_CASE("ResourceManager кеширует файлы", "[resource_manager]")
{
    const std::string test_file = "test_cache.txt";
    create_test_file(test_file, "original\n");

    auto& rm = ResourceManager::instance();
    rm.clear();  // Начинаем с чистого кеша

    auto f1 = rm.get_file(test_file, "r");
    auto f2 = rm.get_file(test_file, "r");

    // Оба указателя должны ссылаться на один объект
    REQUIRE(f1 == f2);
    // Счётчик ссылок должен быть 2 (f1 и f2)
    REQUIRE(f1.use_count() == 2);

    delete_test_file(test_file);
}

/**
 * @brief Тест проверяет принудительное удаление из кеша:
 *        - после evict() новый запрос возвращает другой объект
 */
TEST_CASE("ResourceManager удаляет файл из кеша при evict", "[resource_manager]")
{
    const std::string test_file = "test_evict.txt";
    create_test_file(test_file, "test\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    auto f1 = rm.get_file(test_file, "r");
    rm.evict(test_file);                     // Принудительно удаляем из кеша
    auto f2 = rm.get_file(test_file, "r");

    // Должны быть разные объекты
    REQUIRE(f1 != f2);

    delete_test_file(test_file);
}

/**
 * @brief Тест проверяет потокобезопасность ResourceManager:
 *        - несколько потоков одновременно запрашивают один и тот же файл
 *        - все должны получить один и тот же shared_ptr
 */
TEST_CASE("ResourceManager потокобезопасен", "[resource_manager]")
{
    const std::string test_file = "test_thread.txt";
    create_test_file(test_file, "thread safe\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<FileHandle>> handles(10);

    // Запускаем 10 потоков, каждый получает доступ к одному и тому же файлу
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&rm, test_file, i, &handles]() {
            handles[i] = rm.get_file(test_file, "r");
        });
    }

    // Ждём завершения всех потоков
    for (auto& t : threads) {
        t.join();
    }

    // Все shared_ptr должны указывать на один объект
    for (int i = 1; i < 10; ++i) {
        REQUIRE(handles[0] == handles[i]);
    }

    delete_test_file(test_file);
}
