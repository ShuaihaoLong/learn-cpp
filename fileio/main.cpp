#include <iostream>
#include <fstream>
#include <string>

#include "log.hpp"

std::string ifname = "test.txt";
std::string ofname = "numbered.txt";
int main() {
    std::ifstream ifile("test.txt");
    std::ofstream ofile("numbered.txt");
    if (!ifile) {
        std::cerr << "file open failed\n";
        LOG_ERROR("failed to open test.txt");
        return 1;
    }
    if (!ofile) {
        std::cerr << "Fail to open numbered.txt\n";
        LOG_ERROR("failed to open numbered.txt");
        return 1;
    }
    std::string line;
    int lineNumber = 1;

    LOG_INFO("start");
    while(std::getline(ifile, line)) {
        ofile << std::to_string(lineNumber) + ": " + line << '\n'; 
        lineNumber++;
    }
    LOG_INFO("end");
    return 0;
}
