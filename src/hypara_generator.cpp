#include "hypara_generator.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

namespace {
    const std::string WHITESPACE = " \t\n\r\f\v";

    std::string trim(const std::string& s) {
        size_t start = s.find_first_not_of(WHITESPACE);
        if (start == std::string::npos) return "";
        size_t end = s.find_last_not_of(WHITESPACE);
        return s.substr(start, end - start + 1);
    }

    // [����] ʹ��������ϸ�
    template<typename T>
    std::vector<T> parse_array_string(const std::string& s, const std::string& var_name) {
        std::vector<T> result;
        std::stringstream ss(s);
        std::string item_str;
        while (std::getline(ss, item_str, ',')) {
            std::string trimmed_item = trim(item_str);
            if (trimmed_item.empty()) continue;
            try {
                if constexpr (std::is_same_v<T, int>) {
                    result.push_back(std::stoi(trimmed_item));
                }
                else if constexpr (std::is_same_v<T, double>) {
                    result.push_back(std::stod(trimmed_item));
                }
            }
            catch (const std::exception& e) {
                // [��] �׳����������ĵ���ϸ����
                throw std::invalid_argument(
                    "Invalid element '" + trimmed_item + "' in array '" + var_name + "'"
                );
            }
        }
        return result;
    }
}

namespace BridgeWind {

    void HyparaFile::load() {
        this->variables.clear();

        std::ifstream ifs(filePath);
        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        // --- ��������������ʽ��ʹ��ͳһ���Զ��嶨��� `+` ---

        // ��ʽ 1: string name = "value";
        const std::regex string_regex(R"+(^\s*string\s+([a-zA-Z0-9_]+)\s*=\s*"(.*)"\s*;\s*$)+");

        // ��ʽ 2: type name = value; (���� int, double)
        const std::regex numeric_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*=\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*;\s*$)+");

        // ��ʽ 3: type name[] = [values]; (�����ŵ�����)
        const std::regex array_with_brackets_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*\[\s*\]\s*=\s*\[(.*)\]\s*;\s*$)+");

        // [����] ��ʽ 4: type name[] = values; (�������ŵ�����)
        // ������ʽ����� '=' ֮�� ';' ֮ǰ����������
        const std::regex array_no_brackets_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*\[\s*\]\s*=\s*(.*)\s*;\s*$)+");

        std::string line;
        int line_number = 0;
        while (std::getline(ifs, line)) {
            line_number++;

            // Ԥ����
            size_t comment_pos = line.find("//");
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
            comment_pos = line.find('#');
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
            std::string trimmed_line = trim(line);
            if (trimmed_line.empty()) continue;

            // --- ƥ��������߼� ---
            std::smatch match;

            // �����������ʽ��ƥ���߼��ϲ�����Ϊ���ǵĺ�����������ͬ��
            bool is_array_match = std::regex_match(trimmed_line, match, array_with_brackets_regex) ||
                std::regex_match(trimmed_line, match, array_no_brackets_regex);

            if (is_array_match) {
                // �������������ʽ
                std::string type_str = match[1];
                std::string name = match[2];
                std::string values_str = match[3];
                if (type_str == "int") {
                    variables.emplace_back(name, parse_array_string<int>(values_str, name));
                }
                else { // type_str == "double"
                    variables.emplace_back(name, parse_array_string<double>(values_str, name));
                }
            }
            else if (std::regex_match(trimmed_line, match, string_regex)) {
                // ���� string
                std::string name = match[1];
                std::string value = match[2];
                variables.emplace_back(name, value);
            }
            else if (std::regex_match(trimmed_line, match, numeric_regex)) {
                // ���� int/double ����
                std::string type_str = match[1];
                std::string name = match[2];
                std::string value_str = match[3];
                try {
                    if (type_str == "int") {
                        variables.emplace_back(name, std::stoi(value_str));
                    }
                    else { // type_str == "double"
                        variables.emplace_back(name, std::stod(value_str));
                    }
                }
                catch (const std::out_of_range&) {
                    throw std::runtime_error(
                        "Value out of range in '" + filePath + "' on line " + std::to_string(line_number) +
                        ": -> \"" + trimmed_line + "\""
                    );
                }
            }
            else {
                // ������֪��ʽ��ƥ��ʧ�ܣ��׳�����
                throw std::runtime_error(
                    "Syntax error in '" + filePath + "' on line " + std::to_string(line_number) +
                    ": Invalid statement -> \"" + trimmed_line + "\""
                );
            }
        }
    }

    std::string HyparaVar::toString() const {
        std::stringstream ss;

        std::visit([&ss, this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            // --- �������ͺͱ��������� ---
            std::stringstream name_part;
            if constexpr (std::is_same_v<T, int>) {
                name_part << "int    " << std::left << std::setw(23) << this->name;
            }
            else if constexpr (std::is_same_v<T, double>) {
                name_part << "double " << std::left << std::setw(23) << this->name;
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                name_part << "string " << std::left << std::setw(23) << this->name;
            }
            else if constexpr (std::is_same_v<T, std::vector<int>>) {
                name_part << "int    " << std::left << std::setw(20) << (this->name + "[]");
            }
            else if constexpr (std::is_same_v<T, std::vector<double>>) {
                name_part << "double " << std::left << std::setw(20) << (this->name + "[]");
            }
            ss << name_part.str() << " = ";

            // --- ����ֵ���� ---
            if constexpr (std::is_same_v<T, int>) {
                ss << arg << ";";
            }
            else if constexpr (std::is_same_v<T, double>) {
                // [�����߼�] ��� double �Ƿ�Ϊ����
                if (arg == std::floor(arg)) {
                    // ��������ǿ���� .0 ��β���
                    ss << std::fixed << std::setprecision(1) << arg;
                }
                else {
                    // ������������Ĭ�Ͼ�������Ա������ݶ�ʧ
                    ss << arg;
                }
                ss << ";";
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                ss << std::quoted(arg) << ";";
            }
            else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<double>>) {
                ss << "[";
                // ���������е� double������Ҳ����Ӧ����ͬ���߼�
                for (size_t i = 0; i < arg.size(); ++i) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(arg[i])>, double>) {
                        if (arg[i] == std::floor(arg[i])) {
                            ss << std::fixed << std::setprecision(1) << arg[i];
                        }
                        else {
                            ss << arg[i];
                        }
                    }
                    else {
                        ss << arg[i];
                    }
                    if (i < arg.size() - 1) {
                        ss << ", ";
                    }
                }
                ss << "];";
            }
            }, this->value);

        return ss.str();
    }

    // [�ع�] HyparaVar::print() ��ü����
    void HyparaVar::print() {
        std::cout << this->toString() << std::endl;
    }

    // [�ع�] HyparaFile::print() �����޸ģ������Զ������µ� HyparaVar::print()
    void HyparaFile::print() {
        std::cout << "###################################################################" << std::endl;
        std::cout << "#                 Hypara File Content Preview                 #" << std::endl;
        std::cout << "###################################################################" << std::endl;
        for (auto& var : variables) {
            var.print();
        }
        std::cout << "###################################################################" << std::endl;
    }

    // [�ع�] HyparaFile::saveAs() Ҳ��ü����
    void HyparaFile::saveAs(const std::string& new_path) {
        std::ofstream ofs(new_path);
        if (!ofs.is_open()) {
            throw std::runtime_error("Failed to open file for writing: " + new_path);
        }

        ofs << "// Configuration file generated by BridgeWind::HyparaFile\n\n";

        for (const auto& var : this->variables) {
            ofs << var.toString() << "\n";
        }
    }
}