#pragma once
#ifndef SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>

namespace BridgeWind{
    struct SimulationParameters {
        //  ‰»Îº∏∫Œ
        std::string dxfFilePath;
        double refReNumber = 25.0;
		int maxproc = 4; 

        
        std::string workingDirectory;
    };
}




#endif // SIMULATION_PARAMETERS_H