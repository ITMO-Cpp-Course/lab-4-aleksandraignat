#include "resource_core/file_handle.hpp"

#include <cerrno>  // для errno
#include <cstring> // для std::strlen

namespace lab4::resource {

	// ====================== Конструкторы и деструктор ======================

	FileHandle::FileHandle(const std::string& filename, const std::string& mode) : filename_(filename) {
		// Делегируем открытие методу open()
		open(filename, mode);
	}

	FileHandle::FileHandle(FileHandle&& other) noexcept
		: file_(other.file_) // Забираем указатель
		  ,
		  filename_(std::move(other.filename_)) // Перемещаем строку
	{
		// Обнуляем исходный объект, чтобы он не закрыл файл при своём уничтожении
		other.file_ = nullptr;
	}

	FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
		if (this != &other) { // Защита от самоприсваивания
			close();		  // Закрываем текущий файл, если он открыт

			// Перемещаем ресурс
			file_ = other.file_;
			filename_ = std::move(other.filename_);

			// Обнуляем исходный объект
			other.file_ = nullptr;
		}
		return *this;
	}

	FileHandle::~FileHandle() {
		// Автоматическое освобождение ресурса — главный принцип RAII!
		close();
	}

	// ====================== Открытие и закрытие ======================

	void FileHandle::open(const std::string& filename, const std::string& mode) {
		// Нельзя открыть файл, если уже открыт другой
		if (is_open()) {
			throw ResourceError("FileHandle already open: " + filename_);
		}

		// Пытаемся открыть файл через стандартную функцию C
		file_ = std::fopen(filename.c_str(), mode.c_str());

		if (!file_) {
			// Если не удалось, бросаем исключение с информацией об ошибке
			throw ResourceError("Failed to open file: " + filename + " (errno: " + std::to_string(errno) +
								")");
		}

		filename_ = filename;
	}

	void FileHandle::close() {
		if (file_) {
			std::fclose(file_); // Закрываем файл
			file_ = nullptr;	// Обнуляем указатель
			filename_.clear();	// Очищаем имя файла
		}
	}

	// ====================== Вспомогательные методы ======================

	void FileHandle::check_open() const {
		if (!is_open()) {
			throw ResourceError("FileHandle is not open");
		}
	}

	// ====================== Чтение и запись ======================

	std::string FileHandle::read_line() {
		check_open(); // Убеждаемся, что файл открыт

		char buffer[4096]; // Буфер для чтения строки

		// Читаем одну строку (до '\n' или конца файла)
		if (std::fgets(buffer, sizeof(buffer), file_) == nullptr) {
			// Если достигнут конец файла — возвращаем пустую строку
			if (std::feof(file_)) {
				return "";
			}
			// Иначе — ошибка чтения
			throw ResourceError("Failed to read line from file: " + filename_);
		}

		// Удаляем символ новой строки из конца строки (если он есть)
		size_t len = std::strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
		}

		return std::string(buffer);
	}

	void FileHandle::write_line(const std::string& data) {
		check_open(); // Убеждаемся, что файл открыт

		// Записываем строку с добавлением '\n'
		if (std::fprintf(file_, "%s\n", data.c_str()) < 0) {
			throw ResourceError("Failed to write to file: " + filename_);
		}

		// Сбрасываем буфер, чтобы данные точно попали на диск
		std::fflush(file_);
	}

} // namespace lab4::resource
