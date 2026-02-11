#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

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

//перемножение матриц
std::vector<std::vector<float>> mult_mat(const std::vector<std::vector<float>>& mat1, const std::vector<std::vector<float>>& mat2){
    if(mat1.empty() || mat2.empty() || mat1[0].size() != mat2.size()){
        std::cerr << "Неверные размеры" << std::endl;
        return;
    }

    std::vector<std::vector<float>> C(mat1.size(), std::vector<float>(mat2[0].size()));
    
    for(size_t i = 0; i < mat1.size(); ++i){
        for(size_t j = 0; j < mat2[0].size(); ++j){
            for (size_t k = 0; k < mat2.size(); ++k){
                C[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    
    return C;
    
}

//сохранение матрицы в файл txt
void save_mat(const std::string& filepath, const std::vector<std::vector<float>>& mat) {
    std::ofstream file(filepath);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка открытия файла для записи" << std::endl;
        return;
    }

    for (const auto& row : mat) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << " ";
            }
        }
        file << "\n";
    }

    file.close();
}

int main(int argc, char* argv[]) {
    
    auto start = std::chrono::high_resolution_clock::now(); //начало работы программы


    std::string path1 = argv[1];
    std::string path2 = argv[2];
    std::string pathRes = argv[3];

    std::vector<std::vector<float>> mat1, mat2;


    read_mat(path1, mat1);
    read_mat(path2, mat2);

    std::vector<std::vector<float>> result = mult_mat(mat1, mat2);

    if (!result.empty()) {
        save_mat(pathRes, result);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Время выполнения: " << duration.count() << " мс" << std::endl;
    return 0;
}