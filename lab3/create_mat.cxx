#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <random>

std::vector<double> create_mat(size_t size) {
    std::vector<double> mat(size * size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 100.0);
    for (size_t i = 0; i < size * size; ++i) {
        mat[i] = dis(gen);
    }
    return mat;
}

void save_mat(const std::string& filepath, const std::vector<double>& mat, size_t size) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            file << mat[i * size + j] << (j == size - 1 ? "" : " ");
        }
        file << "\n";
    }
    file.close();
}

int main(int argc, char* argv[]) {
    if (argc < 5) return 1;
    int size1 = std::stoi(argv[1]);
    std::string path1 = argv[2];
    int size2 = std::stoi(argv[3]);
    std::string path2 = argv[4];

    auto mat1 = create_mat(size1);
    auto mat2 = create_mat(size2);

    save_mat(path1, mat1, size1);
    save_mat(path2, mat2, size2);
    return 0;
}