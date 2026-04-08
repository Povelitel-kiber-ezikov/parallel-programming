import os
import subprocess
import re

SIZES = [200, 400, 800, 1200, 1600, 2000]
CORES = [1, 2, 4]

MAT_A = "mat_A.txt"
MAT_B = "mat_B.txt"
MAT_C = "mat_C.txt"
EXE_CREATE = "create_mat.exe"
EXE_MULT = "mult_mat.exe"
SRC_MULT = "mult_mat.cxx"
MD_FILE = "README.md"

MPI_INC = r"C:\Program Files (x86)\Microsoft SDKs\MPI\Include"
MPI_LIB = r"C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64"

def run_cmd(cmd):
    result = subprocess.run(cmd, shell=True, capture_output=True, text=False)
    stdout = result.stdout.decode('utf-8', errors='replace')
    stderr = result.stderr.decode('utf-8', errors='replace')
    if result.returncode != 0:
        return None, stderr
    return stdout, None

def compile_code():
    run_cmd(f"g++ create_mat.cxx -o {EXE_CREATE} -O3")
    mpi_cmd = f'g++ {SRC_MULT} -o {EXE_MULT} -O3 -I"{MPI_INC}" -L"{MPI_LIB}" -lmsmpi'
    out, err = run_cmd(mpi_cmd)
    return out is not None

def main():
    if not compile_code(): return

    results = {size: {} for size in SIZES}

    for size in SIZES:
        print(f"\n--- Test: Size {size} ---")
        run_cmd(f"{EXE_CREATE} {size} {MAT_A} {size} {MAT_B}")

        for cores in CORES:
            if size % cores != 0: continue
            
            print(f"[{cores} cores] Running...", end=" ", flush=True)
            cmd_mpi = f"mpiexec -n {cores} {EXE_MULT} {MAT_A} {MAT_B} {MAT_C}"
            output, err = run_cmd(cmd_mpi)
            
            if output:
                match = re.search(r"Execution time:\s*([\d\.]+)", output)
                if match:
                    time_val = float(match.group(1))
                    results[size][cores] = time_val
                    print(f"OK ({time_val:.2f} ms)")
            else:
                print(f"Error:\n{err}")
                
        run_cmd(f"python check.py {MAT_A} {MAT_B} {MAT_C}")
        

    header = f"| {'Size':<8} | " + " | ".join([f"{c:2} Core(s)" for c in CORES]) + " |"
    separator = f"|{'-' * 10}|" + "|".join([f"{'-' * 11}" for _ in CORES]) + "|"
    
    table_lines = [header, separator]
    for size in SIZES:
        row = f"| {size:<8} | "
        for cores in CORES:
            t = results[size].get(cores, 0)
            row += f"{t:<10.2f} | " if t > 0 else f"{'N/A':<10} | "
        table_lines.append(row)

    final_table = "\n".join(table_lines)
    print("\n" + final_table)

    with open(MD_FILE, "w", encoding="utf-8") as f:
        f.write("# MPI Performance Results\n\n")
        f.write(final_table)
    
    print(f"\nResults saved to {MD_FILE}")

if __name__ == "__main__":
    main()