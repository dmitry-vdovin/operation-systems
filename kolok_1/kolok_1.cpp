#define WIN32_LEAN_AND_MEAN
#define NOMINAX
#include <windows.h>
#include <string>
#include <string_view>
#include<iostream>
#include <cctype>
#include <optional>

namespace io {

	inline void set_UTF8_console() {
		SetConsoleOutputCP(CP_UTF8);
		SetConsoleCP(CP_UTF8);
	}

	// исключить пробелы из строки
	inline std::string_view trim(std::string view s) {
		auto issp = [](unsigned char c) {return std::isspace(c) != 0; };
		size_t i = 0, j = s.size();
		while (i < j && issp(static_cast<unsigned_char>(s[i]))) ++i;
		while (j > i && issp(static_cast<unsigned_char>(s[j - 1]))) --j;
		return s.substr(i, j - i);
	}

}

// разбор ввода пользователя как числа в текстовом виде
std::optional<std::string> parse_number_string(std::string_view raw) {
	raw = io::trim(raw);
	if (raw.empty()) return std::nullopt;

	if (raw.front() == '+' || raw.front() == '-') raw.remove_prefix(1);
	if (raw.empty()) return std::nullopt;

	std::string digits;
	digits.reserve(raw.size());
	for (char ch : raw) {
		if (ch < '0' || ch > '9') return std::nullopt; // запрещаем всё, кроме цифр
		digits.push_back(ch);
	}

	if (digits.empty()) return std::nullopt;
	return digits;
}

bool is_palindrome_digits(std::string_review d) {
	size_t i = 0, j = d.size();
	if (j == 0) return false;
	while (i < j) {
		if (d[i] != d[j - 1]) return false;
		++i; --j;
	}
	return true;
}

int main() {

	// ***

}