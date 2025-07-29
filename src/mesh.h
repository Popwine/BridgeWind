#pragma once
#ifndef MESH_H 
#define MESH_H
#include <vector>
#include <string>
#include "vector2d.h"
#include <unordered_map>
#include "cgnslib.h"
namespace G2C{

enum MeshReverse{
    ReverseNodes,
    ReversePhysicalNames,
    ReverseElements
};
    
struct PhysicalName{


    int dimension;
    //PhysicalName的tag其实就是编号
    int tag;
    std::string name; 
    //emplace_back对于结构体，要在有构造函数的情况下才能使用。

    PhysicalName(int dim, int t, const std::string& n): dimension(dim), tag(t), name(n){}
};


struct Element{
    int id;
    int type;
    int tagNum;
    //tags内容：
    //第一个：物理组tag，
    //第二个：生成来源：生成时来源于哪个边/面，暂时没用
    
    std::vector<int> tags;
    
    std::vector<int> nodesList;
    Element(
        const int i,
        const int tp,
        const int tgnm,
        const std::vector<int>& inTag,
        const std::vector<int>& inNodesList
    ) : id(i),
    type(tp),
    tagNum(tgnm),
    tags(inTag),
    nodesList(inNodesList)
    {}
};

class Mesh{
private:
    std::vector<vector2d<double>> nodes;
    //NodeNumberByVectorIndex的索引是Nodes的索引,值代表gmsh中的序号
    //统一语言：Number代表gmsh里的序号，Index代表vector的索引
    std::vector<int> nodeNumberByVectorIndex;

	//first: node number in gmsh, second: index in nodes vector
    std::unordered_map<int, size_t> nodeNumberToVectorIndex;
    

    std::vector<PhysicalName> physicalNames;

    std::vector<Element> elements;

    std::unordered_map<int, std::vector<size_t>> physTag2ElemIndex;



public:
    void reserveObject(MeshReverse target, size_t number);
    void addNode(int number, double x, double y);
    void printNodes() const;
    int getNodeNumberByVectorIndex(size_t index) const;

    void addPhysicalName(const int dim, const int tag, const std::string& name);
    void printPhysicalNames() const;

    void addElement(
        const int i,
        const int tp,
        const int tgnm,
        const std::vector<int>& inTag,
        const std::vector<int>& inNodesList
    );
    void printElements() const;

    //根据物理标签获取单元的ID。
    std::vector<int> getElementIdsByPhysicalTag(int tag) const;

    Element getElementById(int id);


    //根据物理标签获取单元vector的索引。
    const std::vector<size_t>& getElementIndexesByPhysicalTag(int tag) const;
    Element getElementByIndex(size_t index) const{
        return elements[index];
    }
	vector2d<double> getNodeByIndex(size_t index) const {
		return nodes[index];
	}
    


    
    
    
    size_t getNumElements() const;
	size_t getNumNodes() const;

    size_t getNumQuadElements() const;
    size_t getNumEdgeElements() const;
    size_t getNumPhysicalNames() const;

    void fillCoordinates(std::vector<double>& xCoords, std::vector<double>& yCoords) const;
    
	void fillConnectivity(std::vector<cgsize_t>& quadConnectivity, std::vector<cgsize_t>& edgeConnectivity) const;

    PhysicalName getPhysicalNameByIndex(size_t index) const;
    const std::vector<size_t> getNodeIndexesByPhysicalNameIndex(size_t index) const;

    std::vector<std::vector<size_t>> getQuadElementsIndexConnectivity() const;



};





}


#endif //MESH_H