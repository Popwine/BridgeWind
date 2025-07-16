#include <iostream>
#include <string>
#include <stdexcept>
#include <windows.h>
#include <memory>

#include "geometry.h"
#include "gmsh2cgns.h"
#include "topology_analyzer.h"
#include "geo_generator.h"
#include "app_controller.h"
#include "hypara_generator.h"
#include "bridge_simulation_service.h"


void test_mesh_and_field() {
    G2C::Mesh mesh;
    G2C::GmeshReader reader;
    reader.load("../../../../res/meshes/rect.msh", mesh);
    mesh.printNodes();

    mesh.printPhysicalNames();
    mesh.printElements();
    std::vector<int> findedElementNumber;
    findedElementNumber = mesh.getElementIdsByPhysicalTag(5);
    for (const auto& n : findedElementNumber) {
        std::cout << n << " ";
    }
    std::cout << std::endl;

	auto element = mesh.getElementById(32);
	std::cout << "Element ID: " << element.id << std::endl;
	std::cout << "Element Type: " << element.type << std::endl;
	std::cout << "Element Tags: ";
	for (const auto& tag : element.tags) {
		std::cout << tag << " ";
	}
	std::cout << std::endl;
	std::cout << "Element Nodes: ";
	for (const auto& node : element.nodesList) {
		std::cout << node << " ";
	}
	std::cout << std::endl;


}

void test_gmsh_to_cgns() {
	G2C::convertGmshToCgns(
		"C:/Projects/GeoToMshTest/bridge.msh",
		"../../../../res/meshes/bridge.cgns"
	);

}
void test_dxf_reader(std::string dxf_path) {
	
	BridgeWind::Geometry geometry;
	geometry.loadFromDXF(dxf_path);
	geometry.print();
	//std::vector<BridgeWind::Point> intersectionPoints = geometry.getAllIntersectionPoints();
	std::vector<BridgeWind::Point> intersectionPoints = geometry.getAllIntersectionPointsNoEndPoints();
	for (const auto& point : intersectionPoints) {
		point.printCADCommand();
	}
	BridgeWind::TopologyAnalyzer analyzer(geometry);
	analyzer.analyze();
	analyzer.printLoops();
	BridgeWind::GeoGenerator geoGen(analyzer, "../../../../res/meshes/test.geo");
	geoGen.generateGeoFile();
}
void test_process() {
	BridgeWind::Geometry geometry;
	geometry.loadFromDXF("../../../../res/meshes/Drawing1.dxf");


	BridgeWind::TopologyAnalyzer analyzer(geometry);
	analyzer.analyze();
	analyzer.printLoops();
	BridgeWind::GeoGenerator geoGen(analyzer, "../../../../res/meshes/test.geo");
	geoGen.generateGeoFile();
	geoGen.finalize();
	std::cout << "Geo file generated successfully." << std::endl;
	
	
	BridgeWind::GmshController gmshController(
		"../../../../res/meshes/test.geo",
		"../../../../res/meshes/test.msh"
	);
	
	gmshController.generateMesh();
	



	//gmshController.openOutputMeshFile();
	
	G2C::convertGmshToCgns(
		"../../../../res/meshes/test.msh",
		"../../../../res/meshes/test.cgns"
	);
	std::cout << "CGNS file generated successfully." << std::endl;
	

    
}




void test_hypara() {
	//BridgeWind::HyparaFile hyparaFile("../../../../res/test_case/bin/cfd_para_subsonic.hypara");
	//hyparaFile.load();
	//hyparaFile.print();
	std::string templatePath = "C:/Projects/PHengLEI/phenglei-testcases/B25_unsteady/bin/";
	std::string testPath = "C:/Projects/PHengLEI/phenglei-testcases/B25_unsteady_test_hypara/bin/";
	BridgeWind::HyparaFile boundary_condition(templatePath + "boundary_condition.hypara");
	BridgeWind::HyparaFile cfd_para_subsonic(templatePath + "cfd_para_subsonic.hypara");
	BridgeWind::HyparaFile grid_para(templatePath + "grid_para.hypara");
	BridgeWind::HyparaFile key(templatePath + "key.hypara");
	BridgeWind::HyparaFile partition(templatePath + "partition.hypara");
	BridgeWind::HyparaFile volume_condition(templatePath + "volume_condition.hypara");
	
	cfd_para_subsonic.load();
	grid_para.load();
	key.load();
	partition.load();
	
	
	cfd_para_subsonic.saveAs(testPath + "cfd_para_subsonic.hypara");
	grid_para.saveAs(testPath + "grid_para.hypara");
	key.saveAs(testPath + "key.hypara");
	partition.saveAs(testPath + "partition.hypara");
	

}


void test_service() {
	BridgeWind::SimulationParameters params;
	params.dxfFilePath = "../../../../res/meshes/Drawing1.dxf";
	params.workingDirectory = "C:/Projects/BridgeWind/res/test_service";
	BridgeWind::BridgeSimulationService service;
	service.connectProgressSignal([](const std::string& message) {
		std::cout << "Progress: " << message << std::endl;
		});
	service.connectErrorSignal([](const std::string& message) {
		std::cerr << "Error: " << message << std::endl;
		});
	service.connectFinishedSignal([]() {
		std::cout << "Simulation finished successfully." << std::endl;
		});
	if (service.run(params)) {
		std::cout << "Simulation completed successfully." << std::endl;
	}
	else {
		std::cerr << "Simulation failed." << std::endl;
	}

}
int main() {

	try {
		//test_dxf_reader("../../../../res/meshes/Drawing1.dxf");
		//test_gmsh_to_cgns();
        //test_process();
		//test_hypara();
		test_service();

	}
	catch (const std::runtime_error& e) {
		std::cerr << "Runtime error: " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Unknown error occurred." << std::endl;
		return 1;
	}

	return 0;
}