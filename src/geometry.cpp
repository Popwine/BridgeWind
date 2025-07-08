#include "geometry.h"
#include <iostream>


namespace BridgeWind {
    /*
    * initialize a rectangle with any two points
    */
    Rectangle::Rectangle(const G2C::vector2d<double>& bl, const G2C::vector2d<double>& tr)
        : bottomLeft(bl), topRight(tr) {
        if (bl.x > tr.x || bl.y > tr.y) {
            bottomLeft.x = std::min(bl.x, tr.x);
            bottomLeft.y = std::min(bl.y, tr.y);
            topRight.x = std::max(bl.x, tr.x);
            topRight.y = std::max(bl.y, tr.y);
        }
    }
    Geometry::Geometry() :
        boundingBox(
            G2C::vector2d<double>(0, 0),
            G2C::vector2d<double>(0, 0)),
        isBoundingBoxReal(false),
        epsilon(1e-8)
    {

    }
    void Geometry::addLine(const G2C::vector2d<double>& begin, const G2C::vector2d<double>& end) {
        lines.emplace_back(begin, end);
        if (isBoundingBoxReal) {
            // 更新边界框
            boundingBox.bottomLeft.x = std::min(boundingBox.bottomLeft.x, std::min(begin.x, end.x));
            boundingBox.bottomLeft.y = std::min(boundingBox.bottomLeft.y, std::min(begin.y, end.y));
            boundingBox.topRight.x = std::max(boundingBox.topRight.x, std::max(begin.x, end.x));
            boundingBox.topRight.y = std::max(boundingBox.topRight.y, std::max(begin.y, end.y));
        }
        else {
            // 初始化边界框
            boundingBox = Rectangle(begin, end);
            isBoundingBoxReal = true;
        }
		epsilon = std::max(boundingBox.topRight.x - boundingBox.bottomLeft.x,
			boundingBox.topRight.y - boundingBox.bottomLeft.y) * 1e-8; // 设置 epsilon 为边界框尺寸的 1e-8
    }
    void Geometry::addArc(const G2C::vector2d<double>& center, double radius, double startAngle, double endAngle) {
		Arc arc(center, radius, startAngle, endAngle);
        arcs.push_back(arc);
		if (isBoundingBoxReal) {
			// 更新边界框
			boundingBox.topRight.x = std::max(boundingBox.topRight.x, arc.getBoundingBox().topRight.x);
			boundingBox.topRight.y = std::max(boundingBox.topRight.y, arc.getBoundingBox().topRight.y);
			boundingBox.bottomLeft.x = std::min(boundingBox.bottomLeft.x, arc.getBoundingBox().bottomLeft.x);
			boundingBox.bottomLeft.y = std::min(boundingBox.bottomLeft.y, arc.getBoundingBox().bottomLeft.y);
		}
		else {
			// 初始化边界框
			boundingBox = arc.getBoundingBox();
			isBoundingBoxReal = true;
			
        }
        epsilon = std::max(boundingBox.topRight.x - boundingBox.bottomLeft.x,
            boundingBox.topRight.y - boundingBox.bottomLeft.y) * 1e-8; // 设置 epsilon 为边界框尺寸的 1e-8
    }
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
            addLine(basePoint, secPoint);
        }
        for (const auto& arc : reader.arcs) {
            G2C::vector2d center = { arc.basePoint.x, arc.basePoint.y };
            addArc(center, arc.radious, arc.staangle, arc.endangle);
        }
        for (const auto& circle : reader.circles) {
            G2C::vector2d center = { circle.basePoint.x, circle.basePoint.y };
            addArc(center, circle.radious, 0.0, 2 * PI); // 圆的起始角度为0，结束角度为2π
        }
        for (const auto& polyline : reader.polylines) {
            for (size_t i = 0; i < polyline.vertlist.size() - 1; ++i) {
                const auto& vertex1 = polyline.vertlist[i]->basePoint;
                const auto& vertex2 = polyline.vertlist[i + 1]->basePoint;
                G2C::vector2d start = { vertex1.x, vertex1.y };
                G2C::vector2d end = { vertex2.x, vertex2.y };
                addLine(start, end);
            }
        }
        for (const auto& lwPolyline : reader.lwPolylines) {
            for (size_t i = 0; i < lwPolyline.vertlist.size() - 1; ++i) {
                const auto& vertex1 = lwPolyline.vertlist[i];
                const auto& vertex2 = lwPolyline.vertlist[i + 1];
                G2C::vector2d start = { vertex1->x, vertex1->y };
                G2C::vector2d end = { vertex2->x, vertex2->y };
                addLine(start, end);

            }
        }

    };
    void Geometry::print() const {
        std::cout << "Geometry contains " << lines.size() << " lines and " << arcs.size() << " arcs." << std::endl;
        for (const auto& line : lines) {
            std::cout << "Line from (" << line.begin.x << ", " << line.begin.y << ") to ("
                << line.end.x << ", " << line.end.y << ")" << std::endl;
        }
        for (const auto& arc : arcs) {
            std::cout << "Arc centered at (" << arc.center.x << ", " << arc.center.y
                << ") with radius " << arc.radius
                << ", from angle " << arc.startAngle * 180 / PI
                << "° to angle " << arc.endAngle * 180 / PI
                << "°" << std::endl;
        }
        // bounding box
        std::cout << "Bounding Box: "
            << "Bottom Left=(" << boundingBox.bottomLeft.x << ", " << boundingBox.bottomLeft.y << "), "
            << "Top Right=(" << boundingBox.topRight.x << ", " << boundingBox.topRight.y << ")"
            << std::endl;
    }

    bool Geometry::isEQ(double a, double b) const {
        return std::fabs(a - b) < epsilon;
    }
    bool Geometry::isGT(double a, double b) const {
        return (a - b) > epsilon;
    }
    bool Geometry::isLT(double a, double b) const {
        return (b - a) > epsilon;
    }
    bool Geometry::isGE(double a, double b) const {
        return isEQ(a, b) || isGT(a, b);
    }
    bool Geometry::isLE(double a, double b) const {
        return isEQ(a, b) || isLT(a, b);
    }
    Arc::Arc(const G2C::vector2d<double>& c, double r, double sa, double ea) : center(c), radius(r), startAngle(sa), endAngle(ea) {
        if (radius <= 0) {
            throw std::invalid_argument("Radius must be positive.");
        }
		if (startAngle < 0 || startAngle > 2 * PI || endAngle < 0 || endAngle > 2 * PI) {
			throw std::invalid_argument("Angles must be in the range [0, 2π].");
		}

    };
    bool Arc::isInArc(double angle) const {
        if (angle < 0 || angle > 2 * PI) {
            throw std::invalid_argument("Angle must be in the range [0, 2π].");
        }
        if (startAngle < endAngle) {
			// 正常情况，弧线不穿过0度
			return angle >= startAngle && angle <= endAngle;
		}
        else {
            // 弧线穿过0度
            return angle >= startAngle || angle <= endAngle;
        }
    }
    Rectangle Arc::getBoundingBox() const {
        std::vector<double> xCoords;
		std::vector<double> yCoords;

        // begin
		xCoords.push_back(center.x + radius * cos(startAngle));
		yCoords.push_back(center.y + radius * sin(startAngle));
		// end
		xCoords.push_back(center.x + radius * cos(endAngle));
		yCoords.push_back(center.y + radius * sin(endAngle));
        // 0 degree point (if exit)
		if (
            startAngle > endAngle // 弧线穿过0度处
            || std::fabs(endAngle - startAngle - 2 * PI) < 1e-8 //纯圆
            ) {
			xCoords.push_back(center.x + radius);
			yCoords.push_back(center.y);
		}
		// 90 degree point (if exit)
        if (
			isInArc(PI * 0.5) 
            || std::fabs(endAngle - startAngle - 2 * PI) < 1e-8 //纯圆
            ) {
			xCoords.push_back(center.x);
			yCoords.push_back(center.y + radius);

        }
		// 180 degree point (if exit)
        if (
            isInArc(PI)
            || std::fabs(endAngle - startAngle - 2 * PI) < 1e-8 //纯圆
            ) {
			xCoords.push_back(center.x - radius);
			yCoords.push_back(center.y);

        }
        // 270 degree point (if exit)
        if (
            isInArc(PI * 1.5)
            || std::fabs(endAngle - startAngle - 2 * PI) < 1e-8 //纯圆
            ) {
			xCoords.push_back(center.x);
			yCoords.push_back(center.y - radius);

        }
		// 计算最小和最大坐标
		double minX = *std::min_element(xCoords.begin(), xCoords.end());
		double minY = *std::min_element(yCoords.begin(), yCoords.end());
		double maxX = *std::max_element(xCoords.begin(), xCoords.end());
		double maxY = *std::max_element(yCoords.begin(), yCoords.end());

		return Rectangle(G2C::vector2d<double>(minX, minY), G2C::vector2d<double>(maxX, maxY));
		
    }
    void Line::printCADCommand() const {
		std::cout << "line "
			<< begin.x << "," << begin.y <<" "
			<< end.x << "," << end.y << " "
			<< std::endl << std::endl;
    }
    void Rectangle::printCADCommand() const {
		Line line1(bottomLeft, G2C::vector2d<double>(topRight.x, bottomLeft.y));
		Line line2(G2C::vector2d<double>(topRight.x, bottomLeft.y), topRight);
		Line line3(topRight, G2C::vector2d<double>(bottomLeft.x, topRight.y));
		Line line4(G2C::vector2d<double>(bottomLeft.x, topRight.y), bottomLeft);
		line1.printCADCommand();
		line2.printCADCommand();
		line3.printCADCommand();
		line4.printCADCommand();
    }
}