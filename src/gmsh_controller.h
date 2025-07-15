#ifndef GMSH_CONTROLLER_H
#define GMSH_CONTROLLER_H
#include <iostream>
#include <string>
#include <windows.h>
#include <memory>



namespace BridgeWind {
	class AppController {
	public:
		AppController() = default;
		~AppController() = default;
	
		void simpleExecute(const std::string& command);

		void simpleExecuteWithCreateProcess(const std::string& command);
	};
	class GmshController : public AppController {
	private:
		std::string geoFilePath;
		std::string meshFilePath;
		std::string gmshExecutablePath = "./gmsh.exe"; 
		bool formatMsh2 = true; 
	public:
		GmshController() = delete;
		~GmshController() = default;
		GmshController(const std::string& geoPath, const std::string& meshPath);
		// 设置几何文件路径
		void setGeoFilePath(const std::string& path);
		// 获取几何文件路径
		std::string getGeoFilePath() const;
		// 设置网格文件路径
		void setMeshFilePath(const std::string& path);
		// 获取网格文件路径
		std::string getMeshFilePath() const;
		void generateMesh();
		void openOutputMeshFile();
	};
}


#endif GMSH_CONTROLLER_H