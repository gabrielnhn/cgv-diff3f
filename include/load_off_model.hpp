#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>


class OffModel {
	public:

	// Data member
	std::vector<glm::vec3> vertices;
	std::vector<glm::ivec3> faces;
	
	OffModel(std::string path)
	{
		std::string line;
		std::ifstream myfile (path);
		if (not myfile.is_open())
		{
			std::cout << "Unable to open file"; 
			return;
		}

		// first line

		std::getline(myfile,line);
		// auto success = std::getline(myfile,line);
		
		myfile.close();
	};
};