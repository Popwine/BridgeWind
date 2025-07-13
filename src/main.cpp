#include <iostream>
#include <string>
#include <stdexcept>

#include "geometry.h"
#include "gmsh2cgns.h"
#include "topology_analyzer.h"
#include "geo_generator.h"


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

int main() {

	try {
		test_dxf_reader("../../../../res/meshes/Drawing1.dxf");
		//test_gmsh_to_cgns();
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