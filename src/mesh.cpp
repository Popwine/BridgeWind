#include "mesh.h"
#include <iterator>
#include <iostream>
#include <set>
#include <algorithm>
namespace G2C {

    void Mesh::addNode(int number, double x, double y) {
        nodeNumberByVectorIndex.emplace_back(number);
        nodes.emplace_back(x, y);

        nodeNumberToVectorIndex[number] = nodes.size() - 1;
    }

    void Mesh::printNodes() const{
        for (const auto& v : nodes) {
            std::cout << v << std::endl;
        }
    }
    int Mesh::getNodeNumberByVectorIndex(size_t index)  const {
        return nodeNumberByVectorIndex[index];
    }
    void Mesh::reserveObject(MeshReverse target, size_t number) {

        if (target == MeshReverse::ReverseNodes) {
            nodes.reserve(number);
            nodeNumberByVectorIndex.reserve(number);
        }
        else if (target == MeshReverse::ReversePhysicalNames) {
            physicalNames.reserve(number);

        }
        else if (target == MeshReverse::ReverseElements) {
            elements.reserve(number);

        }
        else {
            throw std::runtime_error("Unexpected reverse target.");
        }

    }
    void Mesh::addPhysicalName(const int dim, const int t, const std::string& n) {
        physicalNames.emplace_back(dim, t, n);
    }

    void Mesh::printPhysicalNames()  const {
        for (const auto& name : physicalNames) {
            std::cout << "dimension: " << name.dimension;
            std::cout << " tag: " << name.tag;
            std::cout << " name: " << name.name << std::endl;
        }
    }
    void Mesh::printElements()  const {
        for (const auto& e : elements) {
            std::cout << "id: " << e.id;
            std::cout << " type: " << e.type;
            std::cout << " tag number: " << e.tagNum;
            std::cout << " tags:";
            for (const auto& t : e.tags) {
                std::cout << " " << t;
            }
            std::cout << " nodes:";
            for (const auto& n : e.nodesList) {
                std::cout << " " << n;
            }
            std::cout << std::endl;
        }
    }

    void Mesh::addElement(
        const int i,
        const int tp,
        const int tgnm,
        const std::vector<int>& inTag,
        const std::vector<int>& inNodesList
    ) {
        elements.emplace_back(i, tp, tgnm, inTag, inNodesList);
        //inTag[0]读取的tags中第一个代表物理组标签
        physTag2ElemIndex[inTag[0]].push_back(elements.size() - 1);
    }

    std::vector<int> Mesh::getElementIdsByPhysicalTag(int tag)  const {
        std::vector<int> elementIds;
        for (const auto& e : elements) {
            if (e.tags.size() > 0 && e.tags[0] == tag) {
                elementIds.push_back(e.id);
            }
        }
        return elementIds;


    }

    const std::vector<size_t>& Mesh::getElementIndexesByPhysicalTag(int tag)  const {
        // std::vector<size_t> elementIndexes;
        // for(size_t i = 0; i < elements.size(); i++){
        //     if(elements[i].tags.size() > 0 && elements[i].tags[0]== tag){
        //         elementIndexes.push_back(i);
        //     }
        // }


        static const std::vector<size_t> empty; // 返回一个静态空 vector 引用以避免返回悬空引用
        auto it = physTag2ElemIndex.find(tag);
        if (it != physTag2ElemIndex.end()) {
            return it->second;
        }
        else {
            return empty;
        }
    }

    Element Mesh::getElementById(int id) {
        for (auto& e : elements) {
            if (e.id == id) {
                return e;
            }
        }
    }


    size_t Mesh::getNumElements()const {
        return elements.size();
    }

    size_t Mesh::getNumNodes() const {
        return nodes.size();
    }


    size_t Mesh::getNumPhysicalNames() const {
        return physicalNames.size();
    }

    PhysicalName Mesh::getPhysicalNameByIndex(size_t index) const {
        return physicalNames[index];
    }

    size_t Mesh::getNumQuadElements() const {
        size_t count = 0;
        for (const auto& e : elements) {
            if (e.type == 3) { // Assuming type 3 corresponds to quadrilateral elements
                count++;
            }
        }
        return count;

    }
    size_t Mesh::getNumEdgeElements()const {
        size_t count = 0;
        for (const auto& e : elements) {
            if (e.type == 1) { // Assuming type 1 corresponds to edge elements
                count++;
            }
        }
        return count;
    }
    void Mesh::fillCoordinates(std::vector<double>& xCoords, std::vector<double>& yCoords) const {
        xCoords.resize(nodes.size());
        yCoords.resize(nodes.size());
        for (size_t i = 0; i < nodes.size(); ++i) {
            xCoords[i] = nodes[i].x;
            yCoords[i] = nodes[i].y;
        }
    }

    void Mesh::fillConnectivity(std::vector<cgsize_t>& quadConnectivity, std::vector<cgsize_t>& edgeConnectivity) const {
        quadConnectivity.clear();
        edgeConnectivity.clear();
        for (const auto& e : elements) {
            if (e.type == 3) { // type 3 corresponds to quadrilateral elements
                for (const auto& node : e.nodesList) {
                    quadConnectivity.push_back(node);
                }
            }
            else if (e.type == 1) { // Assuming type 1 corresponds to edges
                for (const auto& node : e.nodesList) {
                    edgeConnectivity.push_back(node);
                }
            }
        }

        if (quadConnectivity.size() % 4 != 0) {
            throw std::runtime_error("Quad connectivity size is not a multiple of 4.");
        }
        else if (quadConnectivity.size() / 4 != getNumQuadElements()) {
            throw std::runtime_error("Quad connectivity size does not match the number of Quads.");
        }

		if (edgeConnectivity.size() % 2 != 0) {
			throw std::runtime_error("Edge connectivity size is not a multiple of 2.");
		}
		else if (edgeConnectivity.size() / 2 != getNumEdgeElements()) {
			throw std::runtime_error("Edge connectivity size does not match the number of Edges.");
		}


    }
    const std::vector<size_t> Mesh::getNodeIndexesByPhysicalNameIndex(size_t index) const {
        if (index >= physicalNames.size()) {
            throw std::out_of_range("PhysicalName index is out of bounds.");
        }

        int tag = physicalNames[index].tag;
        const std::vector<size_t>& elementIndexes = getElementIndexesByPhysicalTag(tag);

        std::set<size_t> uniqueNodeVectorIndexes; // 使用 set 自动去重
        for (const size_t elemIndex : elementIndexes) {
            // const Element& elem = getElementByIndex(elemIndex); // 理想情况
            const Element elem = getElementByIndex(elemIndex); // 当前情况 (有拷贝)

            for (const int nodeNumber : elem.nodesList) {
                // 使用我们新的映射表进行 O(1) 查找！
                auto it = nodeNumberToVectorIndex.find(nodeNumber);
                if (it == nodeNumberToVectorIndex.end()) {
                    throw std::runtime_error("Node with number " + std::to_string(nodeNumber) + " not found in map.");
                }
                // it->second 就是我们需要的 vector index
                uniqueNodeVectorIndexes.insert(it->second);
            }
        }

        // 从 set 构造 vector 并返回
        return std::vector<size_t>(uniqueNodeVectorIndexes.begin(), uniqueNodeVectorIndexes.end());
    }
    std::vector<std::vector<size_t>> Mesh::getQuadElementsIndexConnectivity() const {
        static std::vector<std::vector<size_t>> empty;
        int numQuad = getNumQuadElements();
        if (numQuad > 0) {
            std::vector<std::vector<size_t>> quadElements;
            for (const auto& e : elements) {
                if (e.type == 3) { // Assuming type 3 corresponds to quadrilateral elements
                    if (e.nodesList.size() != 4) {
                        throw std::runtime_error("Quad element does not have exactly 4 nodes.");
                    }

                    size_t index1 = nodeNumberToVectorIndex.at(e.nodesList[0]);
                    size_t index2 = nodeNumberToVectorIndex.at(e.nodesList[1]);
                    size_t index3 = nodeNumberToVectorIndex.at(e.nodesList[2]);
                    size_t index4 = nodeNumberToVectorIndex.at(e.nodesList[3]);

                    quadElements.emplace_back(std::vector<size_t>{ index1, index2, index3, index4 });

                }
            }
            if (quadElements.size() != numQuad) {
                throw std::runtime_error("Quad elements size does not match the expected number of quad elements.");
            }
            return quadElements;
        }
        else {
            return empty;
        }
    }
}
