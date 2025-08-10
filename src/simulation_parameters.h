#pragma once
#ifndef SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <string>
#include <iostream>
#include <iomanip>
namespace BridgeWind{
	enum class fieldSizeDefineMethod {
		DiameterToMaxSizeRatio, // 
		Default, // 
		UserDefined // 
	};
	enum class geometryDefineMethod {
		BuiltIn, // 
		FromDXF
	};
	enum class viscousModel {
		Laminar, // 
		SSTKOmega
	};


    struct SimulationParameters {
        // 输入几何
        std::string dxfFilePath;
        
		int maxproc = 8; 
		int circumferentialMeshNumber = 180; // 周向网格数
        int radialMeshNumber = 200; // 径向网格数
		double radialMeshGrowthRate = 0.97; // 径向网格增长率
        std::string workingDirectory;

		double filedToSizeRatio = 50.0; // 计算域宽度与几何宽度的比率
		double userDefinedFieldDiameter = 50.0; // 用户定义的计算域直径
		fieldSizeDefineMethod fieldSizeDefineMethod = fieldSizeDefineMethod::Default; // 计算域大小定义方法
		geometryDefineMethod geometryDefineMethod = geometryDefineMethod::BuiltIn;


		double physicalTimeStep = 0.005;
		int    maxSimuStep = 80000;
		int    intervalStepFlow = 2000;
		int    intervalStepPlot = 1000;
		int    intervalStepForce = 10;
		int    intervalStepRes = 10;
		double refDimensionalVelocity = 0.01;       // 来流速度 (m/s)
		double refDimensionalTemperature = 288.15;    // 来流温度 (K)
		double refDimensionalDensity = 1.225;      // 来流密度 (kg/m^3)
		double refReNumber = 100;
		double forceReferenceLength = 1.0;         // 参考长度 (米)
		double forceReferenceArea = 1.0;           // 参考面积 (米^2)
		double forceReferenceLengthSpanWise = 1.0; // 参考翼展 (米)
        double refMachNumber = 4.0-06;

		viscousModel viscousModel = viscousModel::Laminar;

        void print() const {
            std::cout << std::fixed << std::setprecision(6);
            std::cout << "=== Simulation Parameters ===" << std::endl;

            // 字符串参数
            std::cout << "dxfFilePath: " << dxfFilePath << std::endl;
            std::cout << "workingDirectory: " << workingDirectory << std::endl;

            // 整型参数
            std::cout << "maxproc: " << maxproc << std::endl;
            std::cout << "circumferentialMeshNumber: " << circumferentialMeshNumber << std::endl;
            std::cout << "radialMeshNumber: " << radialMeshNumber << std::endl;
            std::cout << "maxSimuStep: " << maxSimuStep << std::endl;
            std::cout << "intervalStepFlow: " << intervalStepFlow << std::endl;
            std::cout << "intervalStepPlot: " << intervalStepPlot << std::endl;
            std::cout << "intervalStepForce: " << intervalStepForce << std::endl;
            std::cout << "intervalStepRes: " << intervalStepRes << std::endl;

            // 浮点型参数
            std::cout << "radialMeshGrowthRate: " << radialMeshGrowthRate << std::endl;
            std::cout << "filedToSizeRatio: " << filedToSizeRatio << std::endl;
            std::cout << "userDefinedFieldDiameter: " << userDefinedFieldDiameter << std::endl;
            std::cout << "physicalTimeStep: " << physicalTimeStep << std::endl;
            std::cout << "refDimensionalVelocity: " << refDimensionalVelocity << std::endl;
            std::cout << "refDimensionalTemperature: " << refDimensionalTemperature << std::endl;
            std::cout << "refDimensionalDensity: " << refDimensionalDensity << std::endl;
            std::cout << "refReNumber: " << refReNumber << std::endl;
            std::cout << "forceReferenceLength: " << forceReferenceLength << std::endl;
            std::cout << "forceReferenceArea: " << forceReferenceArea << std::endl;
            std::cout << "forceReferenceLengthSpanWise: " << forceReferenceLengthSpanWise << std::endl;

            // 枚举参数
            std::cout << "fieldSizeDefineMethod: ";
            switch (fieldSizeDefineMethod) {
            case fieldSizeDefineMethod::DiameterToMaxSizeRatio:
                std::cout << "DiameterToMaxSizeRatio"; break;
            case fieldSizeDefineMethod::Default:
                std::cout << "Default"; break;
            case fieldSizeDefineMethod::UserDefined:
                std::cout << "UserDefined"; break;
            }
            std::cout << std::endl;

            std::cout << "geometryDefineMethod: ";
            switch (geometryDefineMethod) {
            case geometryDefineMethod::BuiltIn:
                std::cout << "BuiltIn"; break;
            case geometryDefineMethod::FromDXF:
                std::cout << "FromDXF"; break;
            }
            std::cout << std::endl;

            std::cout << "viscousModel: ";
            switch (viscousModel) {
            case viscousModel::Laminar:
                std::cout << "Laminar"; break;
            case viscousModel::SSTKOmega:
                std::cout << "SSTKOmega"; break;
            }
            std::cout << std::endl;

            std::cout << "=============================" << std::endl;
        }
    };
}




#endif // SIMULATION_PARAMETERS_H