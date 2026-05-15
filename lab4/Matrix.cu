#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <chrono>
#include <Eigen/Dense>
#include <cuda_runtime.h>

#define CUDA_CHECK(call)                                                \
    do                                                                  \
    {                                                                   \
        cudaError_t err = call;                                         \
        if (err != cudaSuccess)                                         \
        {                                                               \
            std::cerr << "CUDA error в " << __FILE__ << ":" << __LINE__ \
                      << " — " << cudaGetErrorString(err) << std::endl; \
            std::exit(EXIT_FAILURE);                                    \
        }                                                               \
    } while (0)

class Matrix
{
private:
    size_t size_;
    std::vector<double> data_;

public:
    Matrix(size_t rowscols) : size_(rowscols), data_(rowscols * rowscols, 0.0) {}

    // Функция для записи элементов в матрицу
    double &operator()(size_t row, size_t col)
    {
        if (row >= size_ || col >= size_)
        {
            throw std::out_of_range("Индекс вне диапазона");
        }
        return data_[row * size_ + col];
    }

    // Тоже самое, только для чтения
    double operator()(size_t row, size_t col) const
    {
        if (row >= size_ || col >= size_)
        {
            throw std::out_of_range("Индекс вне диапазона");
        }
        return data_[row * size_ + col];
    }

    size_t get_size() const { return size_; }

    void print() const
    {
        for (size_t i = 0; i < size_; ++i)
        {
            for (size_t j = 0; j < size_; ++j)
            {
                std::cout << (*this)(i, j) << "\t";
            }
            std::cout << std::endl;
        }
    }

    double *data() { return data_.data(); }
    const double *data() const { return data_.data(); }
};

__global__ void multiply_matrix_kernel(const double *A,
                                       const double *B,
                                       double *C,
                                       size_t N)
{

    size_t row = blockIdx.y * blockDim.y + threadIdx.y;
    size_t col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < N && col < N)
    {
        double sum = 0.0;
        for (size_t k = 0; k < N; ++k)
            sum += A[row * N + k] * B[k * N + col];
        C[row * N + col] = sum;
    }
}

Matrix multiply_matrix_cuda(const Matrix &A, const Matrix &B, float &gpu_ms_out, int block_dim = 16)
{
    size_t N = A.get_size();
    if (N == 0 || A.get_size() != B.get_size())
        throw std::invalid_argument("Ошибка: матрицы должны быть одного размера!");

    Matrix C(N);
    size_t bytes = N * N * sizeof(double);

    double *d_A = nullptr, *d_B = nullptr, *d_C = nullptr;
    CUDA_CHECK(cudaMalloc(&d_A, bytes));
    CUDA_CHECK(cudaMalloc(&d_B, bytes));
    CUDA_CHECK(cudaMalloc(&d_C, bytes));

    CUDA_CHECK(cudaMemcpy(d_A, A.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_B, B.data(), bytes, cudaMemcpyHostToDevice));

    dim3 blockSize(block_dim, block_dim);
    dim3 gridSize((N + blockSize.x - 1) / blockSize.x,
                  (N + blockSize.y - 1) / blockSize.y);

    cudaEvent_t gpu_start, gpu_stop;
    cudaEventCreate(&gpu_start);
    cudaEventCreate(&gpu_stop);

    cudaEventRecord(gpu_start);
    multiply_matrix_kernel<<<gridSize, blockSize>>>(d_A, d_B, d_C, N);

    cudaEventRecord(gpu_stop);
    CUDA_CHECK(cudaEventSynchronize(gpu_stop));
    CUDA_CHECK(cudaGetLastError());

    cudaEventElapsedTime(&gpu_ms_out, gpu_start, gpu_stop);

    cudaEventDestroy(gpu_start);
    cudaEventDestroy(gpu_stop);

    CUDA_CHECK(cudaMemcpy(C.data(), d_C, bytes, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_A));
    CUDA_CHECK(cudaFree(d_B));
    CUDA_CHECK(cudaFree(d_C));

    return C;
}

Matrix read_matrix_from_stream(std::ifstream &file)
{
    size_t N;
    file >> N;

    Matrix matrix(N);

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            file >> matrix(i, j);
        }
    }

    return matrix;
}

void write_matrix_to_file(const std::string &filename, const Matrix &matrix, long long duration)
{

    std::ofstream file(filename);

    if (!file.is_open())
    {
        throw std::runtime_error("Не удалось открыть файл для записи: " + filename);
    }

    size_t N = matrix.get_size();

    file << N << "\n";

    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < N; ++j)
        {
            file << matrix(i, j) << "\t";
        }
        file << "\n";
    }
    file << "Время работы программы: " << duration << " мкс";
}

Eigen::MatrixXd create_eigen_matrix(const Matrix &m)
{
    // Использует Eigen для проверки матриц на правильность умножения
    size_t N = m.get_size();
    Eigen::MatrixXd result(N, N);
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            result(i, j) = m(i, j);
        }
    }
    return result;
}

bool verify_result(const Matrix &my_matrix, const Eigen::MatrixXd &eigen_matrix)
{
    /*
    Проверяет результаты вычислений с использованием Eigen:
        Возвращает true если вычисления верны
    */
    Eigen::MatrixXd matrix_to_eigen = create_eigen_matrix(my_matrix);

    double diff = (eigen_matrix - matrix_to_eigen).norm();
    if (diff < 1e-9)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int main(int argc, char **argv)
{

    std::string input_path = (argc >= 2) ? argv[1] : "matrix.txt";
    std::string output_path = (argc >= 3) ? argv[2] : "result.txt";

    int block_dim = (argc >= 4) ? std::stoi(argv[3]) : 16;

    std::ifstream file(input_path);
    if (!file.is_open())
    {
        std::cerr << "Не удалось открыть файл: " << input_path << std::endl;
        return 1;
    }

    try
    {
        Matrix A = read_matrix_from_stream(file);
        Matrix B = read_matrix_from_stream(file);
        size_t N = A.get_size();

        Eigen::MatrixXd M = create_eigen_matrix(A);
        Eigen::MatrixXd K = create_eigen_matrix(B);
        Eigen::MatrixXd F = M * K;

        auto start = std::chrono::steady_clock::now();

        float gpu_ms = 0.0f;

        Matrix C = multiply_matrix_cuda(A, B, gpu_ms, block_dim);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        bool ok = verify_result(C, F);

        if (N <= 500)
            write_matrix_to_file(output_path, C, duration.count());

        return ok ? 0 : 2;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 3;
    }
}