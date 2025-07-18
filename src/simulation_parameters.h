#pragma once
#ifndef SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>

namespace BridgeWind{
    struct SimulationParameters {
        // 输入几何
        std::string dxfFilePath;
        double refReNumber = 25.0;
		int maxproc = 8; 
		int circumferentialMeshNumber = 180; // 周向网格数
        int radialMeshNumber = 200; // 径向网格数
		double radialMeshGrowthRate = 0.97; // 径向网格增长率
        std::string workingDirectory;
    };
}




#endif // SIMULATION_PARAMETERS_H