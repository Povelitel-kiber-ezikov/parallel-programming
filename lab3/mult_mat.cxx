#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <mpi.h>
#include <windows.h>

void read_mat(const std::string& filepath, std::vector<double>& mat, int& size) {
    std::ifstream file(filepath);
    if (!file.is_open()) return;
    std::string line;
    size = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        double value;
        while (ss >> value) {
            mat.push_back(value);
        }
        size++;
    }
}

void save_mat(const std::string& filepath, const std::vector<double>& mat, int size) {
    std::ofstream file(filepath);
    if (!file.is_open()) return;
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            file << mat[i * size + j] << (j == size - 1 ? "" : " ");
        }
        file << "\n";
    }
}

void compute_block(int rows, int size, const std::vector<double>& A, const std::vector<double>& B, std::vector<double>& C) {
    for (int i = 0; i < rows; ++i) {
        for (int k = 0; k < size; ++k) {
            for (int j = 0; j < size; ++j) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
}

int main(int argc, char* argv[]) {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    MPI_Init(&argc, &argv);

    int rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc < 4) {
        if (rank == 0) std::cerr << "Использование: " << argv[0] << " <matA> <matB> <matRes>" << std::endl;
        MPI_Finalize();
        return 1;
    }

    std::string path1 = argv[1];
    std::string path2 = argv[2];
    std::string pathRes = argv[3];

    std::vector<double> matA, matB, matC;
    int size = 0;

    // Шаг 1: Подготовка данных (вне замера времени)
    if (rank == 0) {
        read_mat(path1, matA, size);
        read_mat(path2, matB, size);
        if (size == 0) {
            std::cerr << "Ошибка: Матрицы пусты или не найдены." << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        matC.resize(size * size);
    }

    // Шаг 2: Синхронизация и старт таймера
    MPI_Barrier(MPI_COMM_WORLD); 
    double start_time = MPI_Wtime();

    // Передача размера всем процессам
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) matB.resize(size * size);
    MPI_Bcast(matB.data(), size * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    int rows_per_proc = size / world_size;
    std::vector<double> localA(rows_per_proc * size);
    std::vector<double> localC(rows_per_proc * size, 0.0);

    // Рассылка частей матрицы A
    MPI_Scatter(matA.data(), rows_per_proc * size, MPI_DOUBLE, 
                localA.data(), rows_per_proc * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Вычисления
    compute_block(rows_per_proc, size, localA, matB, localC);

    // Сбор результатов
    MPI_Gather(localC.data(), rows_per_proc * size, MPI_DOUBLE, 
               matC.data(), rows_per_proc * size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Шаг 3: Финальная синхронизация и остановка таймера
    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        double duration_ms = (end_time - start_time) * 1000.0;
        std::cout << "Execution time: " << duration_ms << " ms" << std::endl;
        save_mat(pathRes, matC, size);
    }

    MPI_Finalize();
    return 0;
}