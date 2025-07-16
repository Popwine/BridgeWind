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

    // [修正] 使错误处理更严格
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
                // [新] 抛出带有上下文的详细错误
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

        // --- 定义所有正则表达式，使用统一的自定义定界符 `+` ---

        // 格式 1: string name = "value";
        const std::regex string_regex(R"+(^\s*string\s+([a-zA-Z0-9_]+)\s*=\s*"(.*)"\s*;\s*$)+");

        // 格式 2: type name = value; (用于 int, double)
        const std::regex numeric_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*=\s*([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\s*;\s*$)+");

        // 格式 3: type name[] = [values]; (带括号的数组)
        const std::regex array_with_brackets_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*\[\s*\]\s*=\s*\[(.*)\]\s*;\s*$)+");

        // [新增] 格式 4: type name[] = values; (不带括号的数组)
        // 这个表达式捕获从 '=' 之后到 ';' 之前的所有内容
        const std::regex array_no_brackets_regex(R"+(^\s*(int|double)\s+([a-zA-Z0-9_]+)\s*\[\s*\]\s*=\s*(.*)\s*;\s*$)+");

        std::string line;
        int line_number = 0;
        while (std::getline(ifs, line)) {
            line_number++;

            // 预处理
            size_t comment_pos = line.find("//");
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
            comment_pos = line.find('#');
            if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
            std::string trimmed_line = trim(line);
            if (trimmed_line.empty()) continue;

            // --- 匹配与解析逻辑 ---
            std::smatch match;

            // 将两种数组格式的匹配逻辑合并，因为它们的后续处理是相同的
            bool is_array_match = std::regex_match(trimmed_line, match, array_with_brackets_regex) ||
                std::regex_match(trimmed_line, match, array_no_brackets_regex);

            if (is_array_match) {
                // 处理所有数组格式
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
                // 处理 string
                std::string name = match[1];
                std::string value = match[2];
                variables.emplace_back(name, value);
            }
            else if (std::regex_match(trimmed_line, match, numeric_regex)) {
                // 处理 int/double 标量
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
                // 所有已知格式都匹配失败，抛出错误
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

            // --- 构造类型和变量名部分 ---
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

            // --- 构造值部分 ---
            if constexpr (std::is_same_v<T, int>) {
                ss << arg << ";";
            }
            else if constexpr (std::is_same_v<T, double>) {
                // [新增逻辑] 检查 double 是否为整数
                if (arg == std::floor(arg)) {
                    // 是整数，强制以 .0 结尾输出
                    ss << std::fixed << std::setprecision(1) << arg;
                }
                else {
                    // 不是整数，按默认精度输出以避免数据丢失
                    ss << arg;
                }
                ss << ";";
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                ss << std::quoted(arg) << ";";
            }
            else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<double>>) {
                ss << "[";
                // 对于数组中的 double，我们也可以应用相同的逻辑
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

    // [重构] HyparaVar::print() 变得极其简单
    void HyparaVar::print() {
        std::cout << this->toString() << std::endl;
    }

    // [重构] HyparaFile::print() 无需修改，它会自动调用新的 HyparaVar::print()
    void HyparaFile::print() {
        std::cout << "###################################################################" << std::endl;
        std::cout << "#                 Hypara File Content Preview                 #" << std::endl;
        std::cout << "###################################################################" << std::endl;
        for (auto& var : variables) {
            var.print();
        }
        std::cout << "###################################################################" << std::endl;
    }

    // [重构] HyparaFile::saveAs() 也变得极其简单
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