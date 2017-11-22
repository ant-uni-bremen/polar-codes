#include <iostream>
#include <fstream>

int main(int argc, char** argv) {
	std::cout << "This is the factory for fixed decoder creation. argc = " << argc << std::endl;
	if(argc == 2) {
		std::ofstream file(argv[1]);
		if(file.is_open()) {
			std::cout << "File " << argv[1] << " has been created." << std::endl;
			file << "//1234";
			file.close();
		}
	}
	return 0;
}
