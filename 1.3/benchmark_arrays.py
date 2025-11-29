import subprocess
import re
import sys

# -----------------------------
# CONFIGURATION
# -----------------------------
Ns = [100000, 1000000, 10000000]   # N values
J = 4                        # Runs per N
EXECUTABLE = "./arrays"
LOG_FILE = "benchmark_log.txt"

# -----------------------------
# FLOAT EXTRACTION REGEX
# -----------------------------
float_regex = re.compile(r"[-+]?\d*\.\d+")

# -----------------------------
# BUILD + BENCHMARK FUNCTION
# -----------------------------
def build_and_benchmark(better_value, log):
    print(f"Building with BETTER={better_value}...")

    build_cmd = f"make clean && make BETTER={better_value}"
    build_result = subprocess.run(
        build_cmd,
        shell=True,
        capture_output=True,
        text=True
    )

    if build_result.returncode != 0:
        print("BUILD FAILED:")
        print(build_result.stderr)
        sys.exit(1)

    # Header in log file
    log.write(f"BETTER={better_value}\n")

    # Run benchmarks
    for N in Ns:
        serial_sum = 0.0
        parallel_sum = 0.0

        for _ in range(J):
            result = subprocess.run(
                [EXECUTABLE, str(N)],
                capture_output=True,
                text=True
            )

            if result.returncode != 0:
                print(f"Execution failed for N = {N}")
                print(result.stderr)
                sys.exit(1)

            # Extract all floats
            floats = float_regex.findall(result.stdout)

            if len(floats) < 3:
                print("ERROR: Less than 3 floats found in output!")
                print(result.stdout)
                sys.exit(1)

            # Take only the LAST TWO floats
            serial_time = float(floats[-2])
            parallel_time = float(floats[-1])

            serial_sum += serial_time
            parallel_sum += parallel_time

        avg_serial = serial_sum / J
        avg_parallel = parallel_sum / J

        log.write(f"S={avg_serial:.6f} P={avg_parallel:.6f} S/P={(avg_serial / avg_parallel):.6f} E={(avg_serial / avg_parallel / 4):.6f}\n")

# -----------------------------
# MAIN SCRIPT
# -----------------------------
with open(LOG_FILE, "w") as log:
    # First build: BETTER = 0
    build_and_benchmark(0, log)

    # Second build: BETTER = 1
    build_and_benchmark(1, log)

print("Benchmark completed. Results written to:", LOG_FILE)