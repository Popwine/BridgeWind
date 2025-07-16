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
        ~HyparaFile() = default;

        void load();
        void print(); 
        void saveAs(const std::string& new_path);
    };

    class HyparaGenerator {
    public:
        HyparaGenerator() = default;
        ~HyparaGenerator() = default;
    };
}

#endif // HYPARA_GENERATOR_H