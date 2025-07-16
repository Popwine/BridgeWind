#pragma once
#ifndef HYPARA_GENERATOR_H
#define HYPARA_GENERATOR_H

#include <string>
#include <variant>
#include <vector>
#include <filesystem>
#include <iostream>
#include <iomanip>


namespace BridgeWind {
    class HyparaVar {
    public:
        std::string name;
        std::variant<
            int,
            double,
            std::string,
            std::vector<int>,
            std::vector<double>
        > value;

        // --- 构造函数 ---
        HyparaVar() = delete;
        HyparaVar(const std::string& name, int val) : name(name), value(val) {}
        HyparaVar(const std::string& name, double val) : name(name), value(val) {}
        HyparaVar(const std::string& name, const std::string& val) : name(name), value(val) {}
        HyparaVar(const std::string& name, const std::vector<int>& val) : name(name), value(val) {}
        HyparaVar(const std::string& name, const std::vector<double>& val) : name(name), value(val) {}
        std::string toString() const;
        void print();
    };

    class HyparaFile {
    public:
        std::string filePath;
        std::vector<HyparaVar> variables;

        explicit HyparaFile(const std::string& path) : filePath(path) {}; // 使用 explicit 避免隐式转换
		explicit HyparaFile(const std::filesystem::path& path) : filePath(path.string()) {}; // 支持 std::filesystem::path
        ~HyparaFile() = default;

        void load();
        void print(); 
		
        void saveAs(const std::string& new_path) const;
        void saveAs(const std::filesystem::path& new_path) const;
        void save() const { saveAs(this->filePath); }
        auto findVar(const std::string& name) const {
            return std::find_if(this->variables.begin(), this->variables.end(),
                [&name](const HyparaVar& var) {
                    return var.name == name;
                });
        }

        // 非 const 版本，用于修改
        auto findVar(const std::string& name) {
            return std::find_if(this->variables.begin(), this->variables.end(),
                [&name](const HyparaVar& var) {
                    return var.name == name;
                });
        }
        // 基础 get: 获取指定类型的值，如果变量不存在或类型不匹配则抛出异常
        template<typename T>
        T get(const std::string& name) const;

        // 安全 get: 获取指定类型的值，如果变量不存在或类型不匹配，则返回提供的默认值
        template<typename T>
        T get(const std::string& name, T default_value) const;

        // --- 修改变量 ---

        // set: 设置一个变量的值。如果变量已存在，则更新；如果不存在，则创建。
        template<typename T>
        void set(const std::string& name, T value);

        // --- 辅助函数 ---

        // exists: 检查一个变量是否存在
        bool exists(const std::string& name) const;

        // remove: 删除一个变量
        bool remove(const std::string& name);
    };

    class HyparaGenerator {
    public:
        
        HyparaFile cfdParaSubsonic;
        HyparaFile gridPara;
        HyparaFile key;
        HyparaFile partition;

        // 构造函数，加载所有必要的 Hypara 文件
        explicit HyparaGenerator(const std::filesystem::path& templatePath);
        void loadAll();
        void saveAll(const std::filesystem::path& outputPath) const;

    };

    


    // --- API 实现 ---



    template<typename T>
    T HyparaFile::get(const std::string& name) const {
        auto it = findVar(name);
        if (it == this->variables.end()) {
            throw std::runtime_error("Variable '" + name + "' not found.");
        }

        // 使用 std::get_if 进行类型安全的访问
        // 如果 variant 当前存储的不是类型 T，它会返回 nullptr
        if (const T* value_ptr = std::get_if<T>(&(it->value))) {
            return *value_ptr;
        }
        else {
            throw std::runtime_error("Type mismatch for variable '" + name + "'.");
        }
    }

    template<typename T>
    T HyparaFile::get(const std::string& name, T default_value) const {
        auto it = findVar(name);
        if (it == this->variables.end()) {
            return default_value; // 不存在，返回默认值
        }

        if (T* value_ptr = std::get_if<T>(&(it->value))) {
            return *value_ptr; // 类型匹配，返回值
        }
        else {
            return default_value; // 类型不匹配，返回默认值
        }
    }

    template<typename T>
    void HyparaFile::set(const std::string& name, T value) {
        auto it = findVar(name);
        if (it != this->variables.end()) {
            // 变量已存在，直接更新它的 value
            // std::variant 的赋值操作符会处理好一切
            it->value = value;
        }
        else {
            // 变量不存在，在末尾创建一个新的
            this->variables.emplace_back(name, value);
        }
    }
}

#endif // HYPARA_GENERATOR_H