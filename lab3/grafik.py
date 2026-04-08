import matplotlib.pyplot as plt

# Данные
sizes = [200, 400, 800, 1200, 1600, 2000]
core1 = [2.76, 21.09, 288.91, 1107.39, 2373.64, 5124.75]
core2 = [1.74, 18.64, 227.67, 729.58, 1388.22, 2677.22]
core4 = [1.84, 8.31, 143.03, 500.71, 1121.58, 2234.71]

plt.figure(figsize=(10, 6))

# Чистые линии без лишних эффектов
plt.plot(sizes, core1, 'o-', label='1 ядро', linewidth=2)
plt.plot(sizes, core2, 's-', label='2 ядра', linewidth=2)
plt.plot(sizes, core4, '^-', label='4 ядра', linewidth=2)

# Оформление
plt.title('Время выполнения умножения матриц')
plt.xlabel('Размер матрицы (N x N)')
plt.ylabel('Время (мс)')
plt.xticks(sizes)
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend()

plt.tight_layout()
plt.show()