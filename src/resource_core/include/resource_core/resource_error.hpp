#pragma once

#include <stdexcept>
#include <string>

namespace lab4::resource {

	/**
	 * @brief Собственный тип исключений для ошибок при работе с ресурсами
	 *
	 * Наследуется от std::runtime_error, чтобы поддерживать все стандартные
	 * возможности исключений C++ и предоставлять сообщение об ошибке.
	 */
	class ResourceError : public std::runtime_error {
	public:
		/**
		 * @brief Конструктор от std::string
		 * @param message Сообщение об ошибке
		 */
		explicit ResourceError(const std::string& message) : std::runtime_error(message) {}

		/**
		 * @brief Конструктор от C-строки (для удобства)
		 * @param message Сообщение об ошибке
		 */
		explicit ResourceError(const char* message) : std::runtime_error(message) {}
	};

} // namespace lab4::resource
