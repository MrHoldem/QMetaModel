#include <iostream>
#include <yaml-cpp/yaml.h>

int main() {
    try {
        YAML::Node node = YAML::Load("name: test");
        std::cout << "YAML works! name = " << node["name"].as<std::string>() << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}