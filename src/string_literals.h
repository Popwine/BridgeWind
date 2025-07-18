#pragma once
#ifndef STRING_LITERALS_H
#define STRING_LITERALS_H

#include <string>

namespace BridgeWind {
    namespace Strings {
		// 开始
		inline constexpr const char* MeshGenerationStarted = "开始网格生成流程...";
		inline constexpr const char* TopologyAnalysisStarted = "开始拓扑分析...";
		inline constexpr const char* SolverStarting = "开始风雷求解计算...";

		// 正在进行
		inline constexpr const char* LoadingDXF = "正在加载DXF文件: ";
		inline constexpr const char* GeneratingGeoFile = "正在生成Gmsh .geo文件...";
		inline constexpr const char* GeneratingMesh = "正在调用Gmsh生成.msh网格...";
		inline constexpr const char* ConvertingToCgns = "正在将.msh转换为.cgns...";
		inline constexpr const char* HyparaFileGenerateStarted = "正在生成 Hypara 文件...";
		inline constexpr const char* RunningPHengLEIMeshAndPartition = "正在运行 PHengLEI 网格划分和分区...";

		// 错误消息 
		inline constexpr const char* ErrorMeshGeneration = "网格生成失败: ";
		inline constexpr const char* ErrorSolver = "求解器运行失败: ";
		inline constexpr const char* ErrorSolverStarted = "求解器启动失败: ";
		inline constexpr const char* ErrorSimulation = "模拟计算失败: ";
		inline constexpr const char* ErrorOccurred = "发生错误: ";
		inline constexpr const char* MeshGenerationFailed = "网格生成失败: ";
		inline constexpr const char* HyparaFileGenerationFailed = "Hypara 文件生成失败: ";

		
		// 完成
		inline constexpr const char* MeshGenerationComplete = "网格已生成，路径: ";
		inline constexpr const char* TopologyAnalysisComplete = "拓扑分析完成。";
		inline constexpr const char* DXFFileLoaded = "DXF文件已加载。";
		inline constexpr const char* GeoFileGenerated = "Gmsh .geo文件已生成。";
		inline constexpr const char* CgnsFileGenerated = "CGNS文件已生成，路径: ";
		inline constexpr const char* HyparaFileGenerated = "Hypara 文件生成成功";
		inline constexpr const char* PHengLEIMeshAndPartitionCompleted = "PHengLEI 网格划分和分区完成。";
		inline constexpr const char* PHengLEIGridGenerationCompleted = "PHengLEI 网格生成完成。";
		inline constexpr const char* PHengLEIGridPartitionCompleted = "PHengLEI 网格分区完成。";
		inline constexpr const char* SolverStarted = "求解器已启动。";
    }
}

#endif // STRING_LITERALS_H