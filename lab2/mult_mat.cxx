#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <omp.h>

//чтение матрицы из tхt файла
void read_mat(const std::string& filepath, std::vector<std::vector<double>>& mat){
    
    std::ifstream file(filepath);
    
    std::string line;

    while(std::getline(file, line)){
        std::stringstream ss(line);
        std::vector<double> row;
        double value;

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
std::vector<std::vector<double>> mult_mat(const std::vector<std::vector<double>>& mat1, const std::vector<std::vector<double>>& mat2){
    size_t rows = mat1.size();
    size_t cols = mat2[0].size();
    size_t inner = mat2.size();
    
    std::vector<std::vector<double>> C(rows, std::vector<double>(cols, 0.0));
    
    #pragma omp parallel for schedule(static)
    for(size_t i = 0; i < rows; ++i){
        for(size_t j = 0; j < cols; ++j){
            double sum = 0.0; 
            for (size_t k = 0; k < inner; ++k){
                sum += mat1[i][k] * mat2[k][j];
            }
            C[i][j] = sum;
        }
    }
    return C;
}

//сохранение матрицы в файл txt
void save_mat(const std::string& filepath, const std::vector<std::vector<double>>& mat) {
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
    
    std::string path1 = argv[1];
    std::string path2 = argv[2];
    std::string pathRes = argv[3];

    if (argc == 5) {
        omp_set_num_threads(std::stoi(argv[4])); //кол-во потоков
    }

    std::vector<std::vector<double>> mat1, mat2;


    read_mat(path1, mat1);
    read_mat(path2, mat2);

    auto start = std::chrono::high_resolution_clock::now(); //начало работы программы
    std::vector<std::vector<double>> result = mult_mat(mat1, mat2);

    if (!result.empty()) {
        save_mat(pathRes, result);
    }
    
    auto end = std::chrono::high_resolution_clock::now(); //конец работы программы
    std::chrono::duration<double, std::milli> duration = end - start;

    int actual_threads;
    #pragma omp parallel
    {
        #pragma omp single
        actual_threads = omp_get_num_threads();
    }

    std::cout << duration.count() << std::endl;
    return 0;
}
