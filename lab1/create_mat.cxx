#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <random>

//создание случайной матрицы заданного размера
std::vector<std::vector<double>> create_mat(const size_t& size){
    std::vector<std::vector<double>> mat(size, std::vector<double>(size));
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0f, 100.0f);
    
     for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            mat[i][j] = dis(gen);
        }
    }
    return mat;
}

//сохранение матрицы в txt файл
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
    
    int size1 = std::stoi(argv[1]);
    std::string path1 = argv[2];

    int size2 = std::stoi(argv[3]);
    std::string path2 = argv[4];

    std::vector<std::vector<double>> mat1 = create_mat(size1);
    std::vector<std::vector<double>> mat2 = create_mat(size2);

    save_mat(path1, mat1);
    save_mat(path2, mat2);
    return 0;
}