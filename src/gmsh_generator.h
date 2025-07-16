#pragma once
#ifndef GMSH_GENERATOR_H
#define GMSH_GENERATOR_H

#include <stdexcept>
#include <string>
class GmshGeneratorException : public std::runtime_error {
public:
    GmshGeneratorException(const std::string& message) : std::runtime_error(message) {}
};

namespace BridgeWind {
	class GmshGenerator
	{
	public:
		GmshGenerator();
		~GmshGenerator();

	private:

	};


}





#endif // GMSH_GENERATOR_H
