#include <catch2/catch_all.hpp>

#include "resource_core/file_handle.hpp"
#include "resource_core/resource_manager.hpp"

#include <fstream>
#include <thread>
#include <vector>

using namespace lab4::resource;

static void create_test_file(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    file << content;
    file.close();
}

static void delete_test_file(const std::string& filename)
{
    std::remove(filename.c_str());
}

TEST_CASE("FileHandle открывает и автоматически закрывает файл", "[file_handle]")
{
    const std::string test_file = "test_raii.txt";

    {
        FileHandle fh(test_file, "w");
        REQUIRE(fh.is_open());
        fh.write_line("Hello RAII");
    }

    FileHandle fh2(test_file, "r");
    REQUIRE(fh2.is_open());
    auto line = fh2.read_line();
    REQUIRE(line == "Hello RAII");

    delete_test_file(test_file);
}

TEST_CASE("FileHandle бросает исключение при ошибках", "[file_handle]")
{
    const std::string test_file = "test_error.txt";

    REQUIRE_THROWS_AS(FileHandle("/nonexistent/file.txt", "r"), ResourceError);

    FileHandle fh(test_file, "w");
    fh.close();

    REQUIRE_THROWS_AS(fh.read_line(), ResourceError);
    REQUIRE_THROWS_AS(fh.write_line("test"), ResourceError);
    REQUIRE_THROWS_AS(fh.open(test_file, "r"), ResourceError);

    delete_test_file(test_file);
}

TEST_CASE("FileHandle поддерживает перемещение", "[file_handle]")
{
    const std::string test_file = "test_move.txt";

    FileHandle fh1(test_file, "w");
    fh1.write_line("Move test");

    FileHandle fh2 = std::move(fh1);

    REQUIRE_FALSE(fh1.is_open());
    REQUIRE(fh2.is_open());

    delete_test_file(test_file);
}

TEST_CASE("ResourceManager кеширует файлы", "[resource_manager]")
{
    const std::string test_file = "test_cache.txt";
    create_test_file(test_file, "original\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    auto f1 = rm.get_file(test_file, "r");
    auto f2 = rm.get_file(test_file, "r");

    REQUIRE(f1 == f2);
    REQUIRE(f1.use_count() == 2);

    delete_test_file(test_file);
}

TEST_CASE("ResourceManager учитывает режим открытия", "[resource_manager]")
{
    const std::string test_file = "test_modes.txt";
    create_test_file(test_file, "content\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    auto f1 = rm.get_file(test_file, "r");
    auto f2 = rm.get_file(test_file, "w");

    REQUIRE(f1 != f2);

    delete_test_file(test_file);
}

TEST_CASE("ResourceManager удаляет файл из кеша при evict", "[resource_manager]")
{
    const std::string test_file = "test_evict.txt";
    create_test_file(test_file, "test\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    auto f1 = rm.get_file(test_file, "r");
    rm.evict(test_file);
    auto f2 = rm.get_file(test_file, "r");

    REQUIRE(f1 != f2);

    delete_test_file(test_file);
}

TEST_CASE("ResourceManager evict удаляет все режимы файла", "[resource_manager]")
{
    const std::string test_file = "test_evict_all.txt";
    create_test_file(test_file, "test\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    auto f1 = rm.get_file(test_file, "r");
    auto f2 = rm.get_file(test_file, "w");

    rm.evict(test_file);

    auto f3 = rm.get_file(test_file, "r");
    auto f4 = rm.get_file(test_file, "w");

    REQUIRE(f1 != f3);
    REQUIRE(f2 != f4);

    delete_test_file(test_file);
}

TEST_CASE("ResourceManager потокобезопасен", "[resource_manager]")
{
    const std::string test_file = "test_thread.txt";
    create_test_file(test_file, "thread safe\n");

    auto& rm = ResourceManager::instance();
    rm.clear();

    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<FileHandle>> handles(10);

    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&rm, test_file, i, &handles]() { handles[i] = rm.get_file(test_file, "r"); });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    for (int i = 1; i < 10; ++i)
    {
        REQUIRE(handles[0] == handles[i]);
    }

    delete_test_file(test_file);
}
