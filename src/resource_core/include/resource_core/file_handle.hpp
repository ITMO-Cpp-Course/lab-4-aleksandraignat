#pragma once

#include <cstdio> // для std::FILE, std::fopen, std::fclose и т.д.
#include <memory> // для std::shared_ptr (в случае необходимости)
#include <string> // для std::string

#include "resource_core/resource_error.hpp"

namespace lab4::resource {

	/**
	 * @brief RAII-класс для управления файловыми ресурсами
	 *
	 * Класс инкапсулирует файловый указатель std::FILE* и автоматически
	 * закрывает файл при уничтожении объекта. Реализует семантику перемещения,
	 * но запрещает копирование (уникальное владение ресурсом).
	 */
	class FileHandle {
	public:
		/**
		 * @brief Конструктор по умолчанию — создаёт невалидный хендл
		 *
		 * Файл не открыт. Позволяет создать объект без немедленного открытия файла.
		 */
		FileHandle() noexcept = default;

		/**
		 * @brief Конструктор, который сразу открывает файл
		 *
		 * @param filename Имя файла (путь)
		 * @param mode Режим открытия файла ("r", "w", "a", "r+", "w+", и т.д.)
		 * @throws ResourceError Если не удалось открыть файл
		 */
		explicit FileHandle(const std::string& filename, const std::string& mode);

		// Запрещаем копирование — у каждого ресурса должен быть唯一 владелец
		FileHandle(const FileHandle&) = delete;
		FileHandle& operator=(const FileHandle&) = delete;

		/**
		 * @brief Конструктор перемещения
		 *
		 * Передаёт владение ресурсом от другого объекта.
		 * Исходный объект становится невалидным (is_open() == false).
		 *
		 * @param other Объект, из которого перемещается ресурс
		 */
		FileHandle(FileHandle&& other) noexcept;

		/**
		 * @brief Оператор перемещающего присваивания
		 *
		 * @param other Объект, из которого перемещается ресурс
		 * @return FileHandle& Ссылка на текущий объект
		 */
		FileHandle& operator=(FileHandle&& other) noexcept;

		/**
		 * @brief Деструктор — автоматически закрывает файл
		 *
		 * Ресурс освобождается автоматически при уничтожении объекта,
		 * что соответствует принципу RAII.
		 */
		~FileHandle();

		/**
		 * @brief Явное открытие файла (если объект был создан конструктором по умолчанию)
		 *
		 * @param filename Имя файла
		 * @param mode Режим открытия
		 * @throws ResourceError Если файл уже открыт или не удалось открыть
		 */
		void open(const std::string& filename, const std::string& mode);

		/**
		 * @brief Явное закрытие файла
		 *
		 * После закрытия объект становится невалидным.
		 * Можно повторно открыть другой файл через open().
		 */
		void close();

		/**
		 * @brief Получить сырой указатель на FILE
		 *
		 * @return std::FILE* Указатель на файл (может быть nullptr)
		 * @note Используйте осторожно — не закрывайте файл через этот указатель!
		 */
		std::FILE* get() const noexcept { return file_; }

		/**
		 * @brief Проверить, открыт ли файл
		 *
		 * @return true Файл открыт
		 * @return false Файл закрыт или объект пустой
		 */
		bool is_open() const noexcept { return file_ != nullptr; }

		/**
		 * @brief Оператор bool для удобной проверки
		 *
		 * Позволяет писать: if (fh) { ... }
		 */
		explicit operator bool() const noexcept { return is_open(); }

		/**
		 * @brief Прочитать одну строку из файла
		 *
		 * @return std::string Прочитанная строка (без символа '\n')
		 * @throws ResourceError Если файл не открыт или произошла ошибка чтения
		 */
		std::string read_line();

		/**
		 * @brief Записать одну строку в файл
		 *
		 * @param data Строка для записи (автоматически добавляется '\n')
		 * @throws ResourceError Если файл не открыт или произошла ошибка записи
		 */
		void write_line(const std::string& data);

	private:
		std::FILE* file_ = nullptr; // Сырой указатель на файл (ресурс)
		std::string filename_;		// Имя файла (для информативных сообщений об ошибках)

		/**
		 * @brief Вспомогательный метод для проверки, открыт ли файл
		 *
		 * @throws ResourceError Если файл не открыт
		 */
		void check_open() const;
	};

} // namespace lab4::resource
