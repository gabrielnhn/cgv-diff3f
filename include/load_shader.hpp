#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <stdio.h>

class ShaderSourceCode {
	public:

	// Data member
    const char *text;
    std::string str;

    ShaderSourceCode(std::string path)
    {
        // read file to stringstream
        std::ifstream myfile(path);
        if (not myfile.is_open())
		{
			std::cout << "Unable to open file " << path << std::endl; 
			return;
		}
        std::stringstream buffer;
        buffer << myfile.rdbuf();
        str = buffer.str();
        text = str.c_str();
        // printf("%s", text);

    }
};