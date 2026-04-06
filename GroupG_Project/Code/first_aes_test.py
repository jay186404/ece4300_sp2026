import subprocess
import re
#subprocess for Python to run terminal perf +openssl cmds, re = regular expression
Benchmarks= [
    "aes-128-cbc",
    "aes-192-cbc",
    "aes-256-cbc",
    "aes-128-ctr",
    "aes-192-ctr",
    "aes-256-ctr",
    "aes-128-gcm",
    "aes-192-gcm",
    "aes-256-gcm",
]

SECONDS = "3"
PERF_EVENTS = "cycles,instructions"


def extract_value(output, name): #extracts (cycles, instructions)
    """Extract numeric value from perf output."""
    for line in output.splitlines():
        if name in line:
            match = re.search(r"([\d,]+)", line)
            if match:
                return int(match.group(1).replace(",", ""))
    return None


def run_benchmark(test_cypher):  #runs a single AES test
    print("\n" + "=" * 50)
    print(f"Running benchmark: {test_cypher}")
    print("=" * 50)    #prints text out nicely

    cmd = [  # builds the perf stat -e cycles, instructions openssl speed -elapsed -seconds 3 -evp test_cypher command
        "perf", "stat",
        "-e", PERF_EVENTS,
        "openssl", "speed",
        "-elapsed",
        "-seconds", SECONDS,
        "-evp", test_cypher,
    ]

    result = subprocess.run( #runs command defined above, and stores as nmormal text in std
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )

    output = result.stdout

    if result.returncode != 0:  #Foolproof error checking in case of failure to benchmark
        print(f"Error running {test_cypher}")
        print(output)
        return

    # Print full output
    print(output)

    # Extract perf values by calling out def. f(x) to pull values as numerical
    cycles = extract_value(output, "cycles") 
    instructions = extract_value(output, "instructions")

    # Compute IPC
    if cycles is not None and instructions is not None:
        ipc = instructions / cycles
        print(f"\nIPC = instructions / cycles = {instructions} / {cycles} = {ipc:.4f}")  #Calculates our IPC
    else:
        print("\nCould not compute IPC (missing data)")


def main():
    print("Starting AES benchmarks with perf + IPC...\n")

    for test in Benchmarks:
        run_benchmark(test)

    print("\nAll benchmarks complete.")


if __name__ == "__main__":
    main()