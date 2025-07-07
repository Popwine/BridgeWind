#pragma once
#ifndef GMSH2CGNS_H
#define GMSH2CGNS_H
#include <vector>
#include <string>
#include "mesh.h"
#include "gmsh_reader.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include "cgnslib.h" // 引入 CGNS 库的头文件

#ifndef CG_CALL
#define CG_CALL(func) \
    do { \
        int ierr = func; \
        if (ierr != CG_OK) { \
            std::string msg = "CGNS Error at: " + std::string(__FILE__) + \
                              " | Line: " + std::to_string(__LINE__) + \
                              "\n   Failed on call: " + std::string(#func) + \
                              "\n   CGNS Message: " + std::string(cg_get_error()); \
            throw CgnsException(msg); \
        } \
    } while(0)
#endif
class CgnsException : public std::runtime_error {
public:
    CgnsException(const std::string& message) : std::runtime_error(message) {}
};


namespace G2C{
void convertGmshToCgns(
	const std::string& gmshFileName,
	const std::string& cgnsFileName
);

BCType_t autoDetectBCType(std::string name);

bool isSuitForPointRange(std::vector<cgsize_t>& elementList);
void addNumberAll(std::vector<cgsize_t>& elementList, cgsize_t number);
}

#endif //GMSH2CGNS_H