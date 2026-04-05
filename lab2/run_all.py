import subprocess
import os
import sys

def run_command(command, capture=False):
    # Запускаем команду. Если capture=True, вывод идет в переменную, а не на экран
    result = subprocess.run(command, shell=True, capture_output=capture, text=True)
    
    if result.returncode != 0:
        print(f"\n[ОШИБКА] Команда не удалась: {command}")
        if capture: print(f"Детали: {result.stderr}")
        sys.exit(1)
        
    return result.stdout.strip() if capture else None

def main():
    # Набор тестов
    sizes = [100, 200, 300, 500, 800, 1000, 2000]
    threads_list = [1, 2, 4, 8, 16]
    
    prefix = "" if os.name == 'nt' else "./"
    ext = ".exe" if os.name == 'nt' else ""
    
    # 1. Компиляция (выводим один раз)
    print("--- Компиляция исходников ---")
    run_command(f"g++ -O3 create_mat.cxx -o create_mat{ext}")
    run_command(f"g++ -O3 -fopenmp mult_mat.cxx -o mult_mat{ext}")

    # Подготовка таблицы
    header = "| Размер | " + " | ".join([f"{t} потоков" for t in threads_list]) + " |"
    sep = "|---| " + "---| " * len(threads_list)
    md_table = [header, sep]

    print("\n--- Запуск тестов ---")

    for size in sizes:
        # Генерируем один раз для каждого размера
        print(f"Размер {size}x{size}:", end=" ", flush=True)
        run_command(f"{prefix}create_mat{ext} {size} A.txt {size} B.txt")
        
        row = f"| **{size}x{size}** |"
        
        for t in threads_list:
            # Запускаем умножение и забираем время
            output = run_command(f"{prefix}mult_mat{ext} A.txt B.txt C.txt {t}", capture=True)
            
            # Берем только последнюю строку (само число)
            time_val = output.split('\n')[-1].strip()
            
            row += f" {time_val} ms |"
            print(f"[{t}T: {time_val}ms]", end=" ", flush=True)
        
        print(" OK") # Переход на новую строку после всех потоков одного размера
        md_table.append(row)

    # Запись в файл
    with open("README.md", "w", encoding="utf-8") as f:
        f.write("\n".join(md_table))
    
    print("\nГотово! Результаты в файле report.md")

if __name__ == "__main__":
    main()