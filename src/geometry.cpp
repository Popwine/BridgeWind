#include "geometry.h"
#include <iostream>


namespace BridgeWind {
	void DXFReader::printResult() const {
        std::cout << "\n--- DXF Read Results ---" << std::endl;
        std::cout << "Total Circles found: " << circles.size() << std::endl;
        std::cout << "Total Lines found: " << lines.size() << std::endl;
        std::cout << "Total Arcs found: " << arcs.size() << std::endl;
        std::cout << "Total Polylines found: " << polylines.size() << std::endl;
		std::cout << "Total LWPolylines found: " << lwPolylines.size() << std::endl;
        std::cout << "------------------------\n" << std::endl;

        // 打印每个圆的详细信息
        for (size_t i = 0; i < circles.size(); ++i) {
            const auto& circle = circles[i];
            std::cout << "Circle " << i + 1 << ": "
                << "Center=(" << circle.basePoint.x << ", " << circle.basePoint.y << "), "
                << "Radius=" << circle.radious << ", "
                << "Layer='" << circle.layer << "'"
                << std::endl;
        }
        if (!circles.empty()) std::cout << std::endl;


        // 打印每条线的详细信息
        for (size_t i = 0; i < lines.size(); ++i) {
            const auto& line = lines[i];
            std::cout << "Line " << i + 1 << ": "
                << "From=(" << line.basePoint.x << ", " << line.basePoint.y << ") "
                << "To=(" << line.secPoint.x << ", " << line.secPoint.y << "), "
                << "Layer='" << line.layer << "'"
                << std::endl;
        }
        if (!lines.empty()) std::cout << std::endl;

        for (size_t i = 0; i < arcs.size(); ++i) {
            const auto& arc = arcs[i];
            std::cout << "Arc " << i + 1 << ": "
                << "Center=(" << arc.basePoint.x << ", " << arc.basePoint.y << ") "
                << "Angle=(" << arc.staangle / PI * 180 << "°, " << arc.endangle / PI * 180 << "°), "
                << "Layer='" << arc.layer << "'"
                << std::endl;
        }
        
        if (!arcs.empty()) std::cout << std::endl;

        // 打印每条多段线的详细信息
        for (size_t i = 0; i < polylines.size(); ++i) {
            const auto& pline = polylines[i];
            std::cout << "Polyline " << i + 1 << ": "
                << "Vertices=" << pline.vertlist.size() << ", "
                << "Layer='" << pline.layer << "'"
                << (pline.flags & 1 ? ", Closed" : ", Open") // 检查闭合标志位
                << std::endl;

            // 遍历并打印多段线的每个顶点
            for (size_t j = 0; j < pline.vertlist.size(); ++j) {
                const auto& vertex = pline.vertlist[j];
                std::cout << "  - Vertex " << j + 1 << ": (" << vertex->basePoint.x << ", " << vertex->basePoint.y << ")" << std::endl;
            }
            std::cout << std::endl;
            
        }
        for (size_t i = 0; i < lwPolylines.size(); ++i) {
            const auto& lwPline = lwPolylines[i];
            std::cout << "Polyline " << i + 1 << ": "
                << "Vertices=" << lwPline.vertlist.size() << ", "
                << "Layer='" << lwPline.layer << "'"
                << (lwPline.flags & 1 ? ", Closed" : ", Open") // 检查闭合标志位
                << std::endl;

            // 遍历并打印多段线的每个顶点
            for (size_t j = 0; j < lwPline.vertlist.size(); ++j) {
                const auto& vertex = lwPline.vertlist[j];
                std::cout << "  - Vertex " << j + 1 << ": (" << vertex->x << ", " << vertex->y << ")" << std::endl;
            }
            std::cout << std::endl;
            
        }



    }

    void Geometry::loadFromDXF(const std::string& dxfFilePath) {
        DXFReader reader;
        dxfRW fileReader(dxfFilePath.c_str());
        
        
        bool success = fileReader.read(&reader, false);

        // 4. 检查读取是否成功
        if (success) {
            std::cout << "DXF file read successfully!" << std::endl;

            // 读取完成后，所有数据都存储在你的 entity_handler 对象中了
            std::cout << "Found " << reader.circles.size() << " circles." << std::endl;
            std::cout << "Found " << reader.lines.size() << " lines." << std::endl;
            std::cout << "Found " << reader.arcs.size() << " arcs." << std::endl;
            std::cout << "Found " << reader.polylines.size() << " polylines." << std::endl;
            std::cout << "Found " << reader.lwPolylines.size() << " lwPolylines." << std::endl;
            reader.printResult();
        }
        else {
			throw std::runtime_error("Failed to read DXF file: " + dxfFilePath);
        }
		for (const auto& line : reader.lines) {
			G2C::vector2d basePoint = { line.basePoint.x, line.basePoint.y };
			G2C::vector2d secPoint = { line.secPoint.x, line.secPoint.y };
			lines.push_back({ basePoint, secPoint });
		}
        for (const auto& arc : reader.arcs) {
			G2C::vector2d center = { arc.basePoint.x, arc.basePoint.y };
			arcs.push_back({ center, arc.radious, arc.staangle, arc.endangle });
        }
        for (const auto& circle : reader.circles) {
            G2C::vector2d center = { circle.basePoint.x, circle.basePoint.y };
            arcs.push_back({ center, circle.radious, 0.0, 2 * PI }); // 圆的起始角度为0，结束角度为2π
        }
        
    };



}