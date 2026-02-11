#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>


//чтение матрицы из тхт файла
void read_mat(const std::string& filepath, std::vector<std::vector<float>>& mat){
    
    std::ifstream file(filepath);
    
    std::string line;

    while(std::getline(file, line)){
        std::stringstream ss(line);
        std::vector<float> row;
        float value;

        while(ss >> value){
            row.push_back(value);
        }

        if(!row.empty()){
            mat.push_back(row);
        }
    }

    file.close();
}



int main(){
    std::vector<std::vector<float>> A;
    std::vector<std::vector<float>> B;
    return 0;
}