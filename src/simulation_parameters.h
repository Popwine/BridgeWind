#pragma once
#ifnedf SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>



struct SimulationParameters {
    // ���뼸��
    std::string dxfFilePath;


    // ����Ŀ¼�������м��ļ��ͽ����������������
    std::string workingDirectory;
};


#endif // SIMULATION_PARAMETERS_H