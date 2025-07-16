#pragma once
#ifnedf SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>



struct SimulationParameters {
    // 输入几何
    std::string dxfFilePath;


    // 工作目录：所有中间文件和结果都将保存在这里
    std::string workingDirectory;
};


#endif // SIMULATION_PARAMETERS_H