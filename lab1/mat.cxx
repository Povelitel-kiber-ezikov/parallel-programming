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

//перемножение матриц
std::vector<std::vector<float>> mult_mat(const std::vector<std::vector<float>>& A, const std::vector<std::vector<float>>& B){
    if(A.empty() || B.empty() || A[0].size() != B.size()){
        std::cerr << "Неверные размеры" << std::endl;
        return;
    }

    std::vector<std::vector<float>> C;
    
    for(size_t i = 0; i < A.size(); ++i){
        for(size_t j = 0; j < B[0].size(); ++j){
            for (size_t k = 0; k < B.size(); ++k){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    return C;
    

}



int main(){
    std::vector<std::vector<float>> A;
    std::vector<std::vector<float>> B;
    

    return 0;
}