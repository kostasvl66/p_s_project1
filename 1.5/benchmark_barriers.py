import subprocess
import re
import sys

# -----------------------------
# CONFIGURATION
# -----------------------------
Ns = [100, 1000, 10000] # N's to try
Ms = [2, 4, 8, 16] # number of threads to try
J = 4

PROGRAMS = ["./barrier_q1", "./barrier_q2"]
LOG_FILE = "benchmark_log.txt"

# -----------------------------
# FLOAT EXTRACTION REGEX
# -----------------------------
float_regex = re.compile(r"[-+]?\d*\.\d+")

# -----------------------------
# BENCHMARK FUNCTION
# -----------------------------
def benchmark_program(program, log):
    log.write(f"{program[2:]}\n")   # Writes barrier_q1 / barrier_q2

    # Header row
    log.write("N\\M " + " ".join(str(m) for m in Ms) + "\n")

    for N in Ns:
        row_results = []

        for M in Ms:
            total = 0.0

            for _ in range(J):
                result = subprocess.run(
                    [program, str(N), str(M)],
                    capture_output=True,
                    text=True
                )

                if result.returncode != 0:
                    print(f"Execution failed for {program} N={N}, M={M}")
                    print(result.stderr)
                    sys.exit(1)

                floats = float_regex.findall(result.stdout)

                if len(floats) != 1:
                    print("ERROR: Expected exactly ONE float in output!")
                    print(result.stdout)
                    sys.exit(1)

                value = float(floats[0])
                total += value

            avg = total / J
            row_results.append(f"{avg:.6f}")

        # Write row for this N
        log.write(f"{N} " + " ".join(row_results) + "\n")

    log.write("\n")  # Blank line between phases

# -----------------------------
# MAIN SCRIPT
# -----------------------------
with open(LOG_FILE, "w") as log:
    for program in PROGRAMS:
        benchmark_program(program, log)

print("Barrier benchmark completed. Results written to:", LOG_FILE)