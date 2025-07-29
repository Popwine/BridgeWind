#pragma once
#ifndef SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>

namespace BridgeWind{
	enum class filedSizeDefineMethod {
		DiameterToMaxSizeRatio, // 
		Default, // 
		UserDefined // 
	};
    struct SimulationParameters {
        // 输入几何
        std::string dxfFilePath;
        double refReNumber = 25.0;
		int maxproc = 8; 
		int circumferentialMeshNumber = 180; // 周向网格数
        int radialMeshNumber = 200; // 径向网格数
		double radialMeshGrowthRate = 0.97; // 径向网格增长率
        std::string workingDirectory;

		double filedToSizeRatio = 50.0; // 计算域宽度与几何宽度的比率
		double userDefinedFieldDiameter = 50.0; // 用户定义的计算域直径
		filedSizeDefineMethod filedSizeDefineMethod = filedSizeDefineMethod::Default; // 计算域大小定义方法
    };
}




#endif // SIMULATION_PARAMETERS_H