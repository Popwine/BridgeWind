#include <algorithm> // for std::transform
#include <cctype>    // for std::tolower
#include "gmsh2cgns.h"
void G2C::convertGmshToCgns(
	const std::string& gmshFileName,
	const std::string& cgnsFileName
) {
	// For current version of G2C, only 2D Quad meshes are supported.
	G2C::Mesh mesh;
	G2C::GmeshReader reader;
	reader.load(gmshFileName, mesh);
	const char* filename = cgnsFileName.data();
	int fileIndex;
	int baseIndex;

	//create a cgns file
	CG_CALL(cg_open(filename, CG_MODE_WRITE, &fileIndex));

	//write a base node
	CG_CALL(cg_base_write(fileIndex, "Base", 2, 2, &baseIndex));

	cgsize_t zoneSize[3];
	zoneSize[0] = mesh.getNumNodes(); // number of vertices
	zoneSize[1] = mesh.getNumQuadElements(); // number of quads
	zoneSize[2] = 0; // unordered

	//write a zone node A
	int zoneIndexA;
	CG_CALL(cg_zone_write(fileIndex, baseIndex, "A", zoneSize, Unstructured, &zoneIndexA));

	// Create a grid coordinates node
	int gridIndex;
	CG_CALL(cg_grid_write(fileIndex, baseIndex, zoneIndexA, "GridCoordinates", &gridIndex));

	// create a vector of coordinates for X and Y
	std::vector<double> xCoords(mesh.getNumNodes());
	std::vector<double> yCoords(mesh.getNumNodes());

	// fill coordintes vector data from mesh
	mesh.fillCoordinates(xCoords, yCoords);

	// create coordinate arrays for X and Y in cgns
	int coordIndexX, coordIndexY;
    CG_CALL(cg_coord_write(fileIndex, baseIndex, zoneIndexA, RealDouble, 
		"CoordinateX", xCoords.data(), &coordIndexX));
	CG_CALL(cg_coord_write(fileIndex, baseIndex, zoneIndexA, RealDouble,
		"CoordinateY", yCoords.data(), &coordIndexY));

	//
	int coordIndexZ;
	std::vector<double> zCoords(mesh.getNumNodes(), 0.0); // Z coordinates are zero for 2D mesh
	CG_CALL(cg_coord_write(fileIndex, baseIndex, zoneIndexA, RealDouble,
		"CoordinateZ", zCoords.data(), &coordIndexZ)); // No need to store index for Z in 2D mesh

	//
	int currentElementIndex = 0; // notice : CGNS uses 1-based indexing for elements


	// create a list of quads connections
	std::vector<cgsize_t> quadConnectivity;
	std::vector<cgsize_t> edgeConnectivity;
	mesh.fillConnectivity(quadConnectivity, edgeConnectivity);

	int quadElementSectionIndex;
	CG_CALL(cg_section_write(
		fileIndex, baseIndex, zoneIndexA,
		"QuadElements",				// section name
		QUAD_4,						// type		
		currentElementIndex + 1,							// start index
		currentElementIndex + mesh.getNumQuadElements(),	// end index
		0,							// Set to zero if the elements are unsorted.
		quadConnectivity.data(),	// connectivity data
		&quadElementSectionIndex	// section index
	));
	currentElementIndex += mesh.getNumQuadElements(); // Update the current element index

	// create a list of edges connections
	size_t gmshPhysicalNamesCount = mesh.getNumPhysicalNames();
	std::vector<int> edgeElementSectionIndex(gmshPhysicalNamesCount);

	for (size_t i = 0; i < gmshPhysicalNamesCount; i++) {
		if (mesh.getPhysicalNameByIndex(i).dimension != 1) {
			continue; // Skip if the physical name is not for edge elements
		}
		// Get the physical name and the element indexes associated with it
		const auto& physicalName = mesh.getPhysicalNameByIndex(i);
		const auto& elementIds = mesh.getElementIdsByPhysicalTag(physicalName.tag);

		if (elementIds.empty()) {
			continue; // Skip if no elements are associated with this physical name
		}

		std::vector<cgsize_t> edgeConnectivityForSection;
		edgeConnectivityForSection.reserve(elementIds.size() * 2); // Each edge has 2 nodes

		// Iterate through the element IDs and fill the connectivity for edge elements
		for (const auto& elemId : elementIds) {
			const auto& element = mesh.getElementById(elemId);
			if (element.type != 1) { // type 1 corresponds to edge elements
				continue; // Skip if the element is not an edge
			}
			if (element.nodesList.size() != 2) {
				throw std::runtime_error("Edge element must have exactly 2 nodes.");
			}

			// fill the connectivity for the edge element, two nodes per edge
			for (const auto& node : element.nodesList) {
				if (node < 1 || node > mesh.getNumNodes()) {
					throw std::runtime_error("Node index out of bounds in edge element.");
				}
				edgeConnectivityForSection.push_back(node);
			}
		}
		if (edgeConnectivityForSection.size() % 2 != 0) {
			throw std::runtime_error("Edge connectivity size is not a multiple of 2.");
		}
		if (edgeConnectivityForSection.size() / 2 != elementIds.size()) {
			throw std::runtime_error("Edge connectivity size does not match the number of elements.");
		}
		
		
		CG_CALL(cg_section_write(
			fileIndex, baseIndex, zoneIndexA,
			physicalName.name.c_str(),										// section name
			BAR_2,															// type		
			currentElementIndex + 1,										// start index
			currentElementIndex + edgeConnectivityForSection.size() / 2,	// end index
			0,									// Set to zero if the elements are unsorted.
			edgeConnectivityForSection.data(),	// connectivity data
			&edgeElementSectionIndex[i]			// section index
		));
		
		currentElementIndex += edgeConnectivityForSection.size() / 2; // Update the current element index

	}

	if (currentElementIndex != mesh.getNumElements()) {
		throw std::runtime_error("Current element index does not match the number of elements in the mesh.");
	}


	// Write the physical names
	std::unordered_map<std::string, int> family_indexes;

	for (size_t i = 0; i < gmshPhysicalNamesCount; ++i) {
		// 1. �Ȼ�ȡ�����������������֣������ظ�����
		const auto& physicalName = mesh.getPhysicalNameByIndex(i);
		const std::string& name = physicalName.name;
		if (physicalName.dimension == 0 || physicalName.dimension == 3) {
			continue; // ���������������
		}
		// 2. ����Ƿ��Ѿ������������ֵ���
		if (family_indexes.count(name)) {
			continue;
		}

		// 3. ����Ƿ��нڵ����
		if (mesh.getNodeIndexesByPhysicalNameIndex(i).empty()) {
			std::cerr << "Warning: Skipping family '" << name << "' because it has no associated nodes." << std::endl;
			continue;
		}

		// 4. ʹ��һ����ʱ�����������������ͼ������
		// ����Ǵ���������� 2D �����飬����ֻ�����壬������BC
		if (physicalName.dimension == 2) {
			int dummy_family_index;
			CG_CALL(cg_family_write(fileIndex, baseIndex, name.c_str(), &dummy_family_index));
			family_indexes[name] = dummy_family_index;
			continue; // ������һ��������
		}
		int family_index = 0;
		CG_CALL(cg_family_write(
			fileIndex,
			baseIndex,
			name.c_str(),      // ʹ�� .c_str()
			&family_index  // ������ʱ�����ĵ�ַ
		));

		// 5. ����ȡ������������ map
		family_indexes[name] = family_index;

		int fambcIndex;
		CG_CALL(cg_fambc_write(
			fileIndex,
			baseIndex,
			family_index, // ʹ�õ�һ�����ص�����
			"FamBC",    // ��� FamilyBC_t �ڵ�����֣������Զ���
			autoDetectBCType(name),   // �ؼ��������ﶨ����������
			&fambcIndex
		));

		auto elementIndexes = mesh.getElementIndexesByPhysicalTag(physicalName.tag);
		for (auto& e : elementIndexes) {
			e = e + 1;
		}
		std::vector<cgsize_t> elementList(elementIndexes.begin(), elementIndexes.end());
		addNumberAll(elementList, static_cast<cgsize_t>(mesh.getNumQuadElements()));
		int BCIndex;
		//-----------------------------------------------------
		bool isPointRange = isSuitForPointRange(elementList); // ���Ԫ���б��Ƿ��ʺϵ㷶Χ

		if (isPointRange) {
			std::vector<cgsize_t> elementRange{
				elementList.front(), // ��һ��Ԫ��
				elementList.back()   // ���һ��Ԫ��
			};
			CG_CALL(cg_boco_write(
				fileIndex, baseIndex, zoneIndexA,
				physicalName.name.c_str(),			//boconame
				FamilySpecified,					//bocotype
				PointRange,							//ptset_type 
				elementRange.size(),
				elementRange.data(),
				&BCIndex
			));
		}
		else {
			CG_CALL(cg_boco_write(
				fileIndex, baseIndex, zoneIndexA,
				physicalName.name.c_str(),			//boconame
				FamilySpecified,					//bocotype
				PointList,							//ptset_type 
				elementList.size(),
				elementList.data(),
				&BCIndex
			));
		}
		
		//-------------------------------------------------------
		CG_CALL(cg_goto(
			fileIndex, baseIndex,
			"Zone_t", zoneIndexA,
			"ZoneBC_t", 1,      // ͨ��ֻ��һ�� ZoneBC ���������������� 1
			"BC_t", BCIndex,   // ʹ����һ�����ص���������λ
			"end"
		));
		CG_CALL(cg_famname_write(name.c_str()));
		CG_CALL(cg_gridlocation_write(EdgeCenter));

		

	}
	
	CG_CALL(cg_goto(fileIndex, baseIndex, "end")); // �ص�Base�ڵ�
	CG_CALL(cg_dataclass_write(Dimensional));
	CG_CALL(cg_units_write(Kilogram, Meter, Second, Kelvin, Radian));

	CG_CALL(cg_close(fileIndex));
} 

BCType_t G2C::autoDetectBCType(std::string name){ // Ϊ�˺����ĺ���ǩ������һ��

	// 1. �������ַ���ȫ��ת��ΪСд�����ڲ����ִ�Сд�ıȽ�
	std::transform(name.begin(), name.end(), name.begin(),
		[](unsigned char c) { return std::tolower(c); });

	// 2. �����ȼ����йؼ���ƥ��

	// --- �����صı߽� ---
	if (name.find("inlet") != std::string::npos ||
		name.find("inflow") != std::string::npos) {
		return BCInflow;//BCInflow; // Ĭ�Ϸ�����������ڣ�������������
	}

	// --- ������صı߽� ---
	if (name.find("outlet") != std::string::npos ||
		name.find("outflow") != std::string::npos) {
		return BCOutflow;//BCOutflow; // Ĭ�Ϸ��������ٳ���
	}

	// --- �ԳƱ߽� ---
	// ������Խ�һ��ϸ��
	if (name.find("symmetry") != std::string::npos) {
		// �����Ҫ���ּ�����Գƣ����Լ�������ؼ���
		if (name.find("polar") != std::string::npos || name.find("axis") != std::string::npos) {
			return BCSymmetryPolar;
		}
		// ����Ĭ��Ϊ�Գ�ƽ��
		return BCSymmetryPlane;
	}

	// --- ������صı߽� ---
	// ���ƥ��Ӧ�÷������/����֮����Ϊ������ "inlet_wall" ����������
	if (name.find("wall") != std::string::npos||
		name.find("solid surface") != std::string::npos) {
		// ���Ը��������ؼ���ϸ����������
		if (name.find("inviscid") != std::string::npos || name.find("slip") != std::string::npos) {
			return BCWallInviscid; // ��ճ/���Ʊ���
		}
		if (name.find("isothermal") != std::string::npos) {
			return BCWallViscousIsothermal; // ����ճ�Ա���
		}
		if (name.find("heatflux") != std::string::npos) {
			return BCWallViscousHeatFlux; // �����ܶȱ���
		}
		// Ĭ�Ϸ���ͨ��ճ�Ա���
		return BCWall; // ճ�Ա���BCWallViscous���߸�ͨ�õ� BCWall
	}

	// --- Զ���߽� ---
	if (name.find("farfield") != std::string::npos ||
		name.find("far-field") != std::string::npos) {
		return BCFarfield;
	}

	// --- �����Ա߽� ---
	if (name.find("periodic") != std::string::npos || name.find("cyclic") != std::string::npos) {
		// ע�⣺�����Ա߽���CGNS���и����ӵĶ��壬����ֻ��������
		// ʵ��Ӧ������Ҫ�� cg_conn_periodic_write �Ⱥ�������任��ϵ
		return BCTypeUserDefined; // �����Ա߽�ͨ����ֱ���� BCType ����
	}


	// 3. ������йؼ��ʶ�δƥ���ϣ�����һ��Ĭ��ֵ
	return BCTypeUserDefined;
}

bool G2C::isSuitForPointRange(std::vector<cgsize_t>& elementList) {
	// ���Ԫ���б��Ƿ�Ϊ��
	if (elementList.empty()) {
		throw std::runtime_error("Element list is empty.");
	}
	for (int i = 0; i < elementList.size() - 1; ++i) {
		// �������Ԫ���Ƿ�����
		if (elementList[i] + 1 != elementList[i + 1]) {
			return false; // ����в�������Ԫ�أ����� false
		}
	}
	return true; // ������м�鶼ͨ�������� true
}

void G2C::addNumberAll(std::vector<cgsize_t>& elementList, cgsize_t number) {
	for (auto& elem : elementList) {
		elem += number; // ��ÿ��Ԫ�ص�ֵ����ָ���� number
	}
}
