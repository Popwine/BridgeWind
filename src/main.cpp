#include <iostream>
#include <string>
#include <stdexcept>

#include "geometry.h"
#include "gmsh2cgns.h"


/*
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


	




}*/

void test_gmsh_to_cgns() {
	G2C::convertGmshToCgns(
		"C:/Projects/GeoToMshTest/rect.msh",
		"../../../../res/meshes/rect.cgns"
	);
}
void test_dxf_reader(std::string dxf_path) {
	
	
	BridgeWind::DXFReader entity_handler;

	// 2. 创建 libdxfrw 的主力读取器对象，并把文件名传给它。
	dxfRW file_reader(dxf_path.c_str());

	// 3. 调用主力读取器的 read() 方法，
	//    并把你的处理器对象的地址 (&entity_handler) 传给它。
	//    这样，file_reader 在解析文件时，就会把数据回调给 entity_handler。
	std::cout << "Starting to read DXF file: " << dxf_path << std::endl;
	bool success = file_reader.read(&entity_handler, false);

	// 4. 检查读取是否成功
	if (success) {
		std::cout << "DXF file read successfully!" << std::endl;

		// 读取完成后，所有数据都存储在你的 entity_handler 对象中了
		std::cout << "Found " << entity_handler.circles.size() << " circles." << std::endl;
		std::cout << "Found " << entity_handler.lines.size() << " lines." << std::endl;
		std::cout << "Found " << entity_handler.polylines.size() << " polylines." << std::endl;
		entity_handler.printResult();
	}
	else {
		std::cerr << "Failed to read DXF file." << std::endl;
	}
}

int main() {

	try {
		test_dxf_reader("../../../../res/meshes/Drawing1.dxf");
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