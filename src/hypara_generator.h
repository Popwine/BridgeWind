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

        // --- ���캯�� ---
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

        explicit HyparaFile(const std::string& path) : filePath(path) {}; // ʹ�� explicit ������ʽת��
		explicit HyparaFile(const std::filesystem::path& path) : filePath(path.string()) {}; // ֧�� std::filesystem::path
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

        // �� const �汾�������޸�
        auto findVar(const std::string& name) {
            return std::find_if(this->variables.begin(), this->variables.end(),
                [&name](const HyparaVar& var) {
                    return var.name == name;
                });
        }
        // ���� get: ��ȡָ�����͵�ֵ��������������ڻ����Ͳ�ƥ�����׳��쳣
        template<typename T>
        T get(const std::string& name) const;

        // ��ȫ get: ��ȡָ�����͵�ֵ��������������ڻ����Ͳ�ƥ�䣬�򷵻��ṩ��Ĭ��ֵ
        template<typename T>
        T get(const std::string& name, T default_value) const;

        // --- �޸ı��� ---

        // set: ����һ��������ֵ����������Ѵ��ڣ�����£���������ڣ��򴴽���
        template<typename T>
        void set(const std::string& name, T value);

        // --- �������� ---

        // exists: ���һ�������Ƿ����
        bool exists(const std::string& name) const;

        // remove: ɾ��һ������
        bool remove(const std::string& name);
    };

    class HyparaGenerator {
    public:
        
        HyparaFile cfdParaSubsonic;
        HyparaFile gridPara;
        HyparaFile key;
        HyparaFile partition;

        // ���캯�����������б�Ҫ�� Hypara �ļ�
        explicit HyparaGenerator(const std::filesystem::path& templatePath);
        void loadAll();
        void saveAll(const std::filesystem::path& outputPath) const;

    };

    


    // --- API ʵ�� ---



    template<typename T>
    T HyparaFile::get(const std::string& name) const {
        auto it = findVar(name);
        if (it == this->variables.end()) {
            throw std::runtime_error("Variable '" + name + "' not found.");
        }

        // ʹ�� std::get_if �������Ͱ�ȫ�ķ���
        // ��� variant ��ǰ�洢�Ĳ������� T�����᷵�� nullptr
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
            return default_value; // �����ڣ�����Ĭ��ֵ
        }

        if (T* value_ptr = std::get_if<T>(&(it->value))) {
            return *value_ptr; // ����ƥ�䣬����ֵ
        }
        else {
            return default_value; // ���Ͳ�ƥ�䣬����Ĭ��ֵ
        }
    }

    template<typename T>
    void HyparaFile::set(const std::string& name, T value) {
        auto it = findVar(name);
        if (it != this->variables.end()) {
            // �����Ѵ��ڣ�ֱ�Ӹ������� value
            // std::variant �ĸ�ֵ�������ᴦ���һ��
            it->value = value;
        }
        else {
            // ���������ڣ���ĩβ����һ���µ�
            this->variables.emplace_back(name, value);
        }
    }
}

#endif // HYPARA_GENERATOR_H