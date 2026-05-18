
#include "resource_core/resource_manager.hpp"

namespace lab4::resource {

	// Синглтон
	ResourceManager& ResourceManager::instance() {
		// Статическая локальная переменная
		static ResourceManager manager;
		return manager;
	}

	// Управление кешем

	std::shared_ptr<FileHandle> ResourceManager::get_file(const std::string& filename, const std::string& mode) {
		// Блокируем мьютекс для потокобезопасного доступа к кешу
		std::lock_guard<std::mutex> lock(mutex_);

		// Ищем файл в кеше
		auto it = cache_.find(filename);

		if (it != cache_.end()) {
			// Пытаемся получить shared_ptr из weak_ptr
			auto shared = it->second.lock();

			if (shared) {
				// возвращаем ресурс
				return shared;
			}
			else {
				// Удаляем устаревшую запись из кеша
				cache_.erase(it);
			}
		}

		// Создаём новый ресурс (открываем файл)
		auto file = std::make_shared<FileHandle>(filename, mode);

		// Сохраняем weak_ptr в кеш
		cache_[filename] = file;

		return file;
	}

	void ResourceManager::evict(const std::string& filename) {
		std::lock_guard<std::mutex> lock(mutex_);
		cache_.erase(filename);
	}

	void ResourceManager::clear() {
		std::lock_guard<std::mutex> lock(mutex_);
		cache_.clear();
	}

}
