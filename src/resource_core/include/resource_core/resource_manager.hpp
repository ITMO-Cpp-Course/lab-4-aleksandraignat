#pragma once

#include <memory>          // для std::shared_ptr, std::weak_ptr
#include <mutex>           // для потокобезопасности
#include <string>          // для std::string
#include <unordered_map>   // для хранения кеша

#include "resource_core/file_handle.hpp"

namespace lab4::resource {

/**
 * @brief Менеджер ресурсов с кешированием
 *
 * Позволяет разделять доступ к одним и тем же файлам через shared_ptr.
 * Использует weak_ptr в кеше, чтобы ресурсы автоматически удалялись,
 * когда на них больше нет внешних ссылок.
 *
 * Реализован как синглтон (один экземпляр на всю программу).
 * Потокобезопасен благодаря использованию std::mutex.
 */
class ResourceManager {
public:
    /**
     * @brief Получить единственный экземпляр менеджера (синглтон)
     *
     * @return ResourceManager& Ссылка на экземпляр
     */
    static ResourceManager& instance();

    // Запрещаем копирование и перемещение для синглтона
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    /**
     * @brief Получить доступ к файлу с кешированием
     *
     * Если файл уже есть в кеше и на него есть живая ссылка,
     * возвращает существующий shared_ptr.
     * Иначе открывает файл и сохраняет в кеше.
     *
     * @param filename Имя файла
     * @param mode Режим открытия (по умолчанию "r" — чтение)
     * @return std::shared_ptr<FileHandle> Разделяемый указатель на файл
     * @throws ResourceError Если не удалось открыть файл
     */
    std::shared_ptr<FileHandle> get_file(const std::string& filename, const std::string& mode = "r");

    /**
     * @brief Принудительно удалить файл из кеша
     *
     * @param filename Имя файла
     */
    void evict(const std::string& filename);

    /**
     * @brief Очистить весь кеш
     */
    void clear();

private:
    // Приватный конструктор — синглтон
    ResourceManager() = default;

    /**
     * @brief Кеш ресурсов
     *
     * weak_ptr не продлевает время жизни объекта, поэтому файл автоматически
     * закроется, когда все shared_ptr на него будут уничтожены.
     */
    std::unordered_map<std::string, std::weak_ptr<FileHandle>> cache_;

    std::mutex mutex_;  // Для потокобезопасности
};

} // namespace lab4::resource
