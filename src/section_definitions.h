#pragma once
#ifndef SECTION_DEFINITIONS_H
#define SECTION_DEFINITIONS_H

#include <QObject>
#include <QString>
#include <QList>

// 1. 使用枚举定义稳定的、与语言无关的截面ID
class SectionTypes {
    Q_GADGET // 使枚举能更好地被Qt元对象系统识别
public:
    enum Type {
        Undefined = -1,
        Circle = 100,  
        Rectangle = 200, 
        ChamferedRectangle = 300,
        StreamlinedBoxGirder = 400,
        CantileverBoxGirder = 401
    };
    Q_ENUM(Type) // 注册枚举
private:
    SectionTypes() {}
};

// 2. 定义一个参数的结构体
struct ParameterDef {
    const char* key;     // 翻译键 (e.g., "param_total_width")，稳定，不翻译
    const char* comment; // 新增：自然语言注释
    double defaultValue; // 默认值
};

// 3. 定义一个完整截面类型的结构体
struct SectionDef {
    SectionTypes::Type id;          // 唯一的、稳定的枚举ID
    const char* nameKey;            // 截面名称的翻译键 (e.g., "section_rectangle")
    const char* nameComment; // 新增：自然语言注释
    QList<ParameterDef> parameters; // 该截面拥有的所有参数列表
};

#endif // SECTION_DEFINITIONS_H