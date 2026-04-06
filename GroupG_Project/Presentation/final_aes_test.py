import subprocess
import re
import time
import board
import busio
from adafruit_ina219 import INA219

# subprocess for Python to run terminal perf + openssl cmds, re = regular expression, time for time lol
Benchmarks = [
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

def extract_value(output, name):  # extracts (cycles, instructions)
    for line in output.splitlines():
        if name in line:
            match = re.search(r"([\d,]+)", line)
            if match:
                return int(match.group(1).replace(",", ""))
    return None

def get_temperature(): #takes temperature readings from internal rPI
    output = subprocess.run(
        ["vcgencmd", "measure_temp"],
        capture_output=True,
        text=True
    ).stdout.strip()
    return float(output.replace("temp=", "").replace("'C", ""))

def get_ina_readings(ina): #Gets the Psu voltage and current
    return ina.bus_voltage, ina.current

def run_benchmark(test_cipher, ina):
    print("\n" + "=" * 50)
    print(f"Running benchmark: {test_cipher}")
    print("=" * 50)

    cmd = [
        "perf", "stat",
        "-e", PERF_EVENTS,
        "openssl", "speed",
        "-elapsed",
        "-seconds", SECONDS,
        "-evp", test_cipher,
    ]

    start = time.time()
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )

    output = ""
    for line in process.stdout:
        output += line
        if any(skip in line.lower() for skip in ["version:", "built on:", "options:", "compiler:", "cpuinfo"] ):
            continue
        
        print(line, end="")

    process.wait()
    elapsed = time.time() - start

    cycles = extract_value(output, "cycles")
    instructions = extract_value(output, "instructions")

    if cycles is not None and instructions is not None:
        ipc = instructions / cycles
        print(f"\nIPC = instructions / cycles = {instructions} / {cycles} = {ipc:.4f}")
    else:
        print("\nCould not compute IPC (data be missin)")

    temp = get_temperature()
    voltage, current = get_ina_readings(ina)

    print(f"Temperature = {temp:.2f} C")
    print(f"Voltage = {voltage:.2f} V")
    print(f"Current = {current:.2f} mA")
    print(f"Total Time = {elapsed:.2f} seconds")

def main():
    print("Starting AES benchmarks with perf + IPC + temperature + INA219...\n")

    # Initialize INA219
    i2c = busio.I2C(board.SCL, board.SDA)
    ina = INA219(i2c)

    # Initial readings
    start_temp = get_temperature()
    start_voltage, start_current = get_ina_readings(ina)

    print("Initial Sensor Readings:")
    print(f"Temperature = {start_temp:.2f} C" if start_temp is not None else "Temperature = N/A")
    print(f"Voltage = {start_voltage:.2f} V")
    print(f"Current = {start_current:.2f} mA")
    print()

    for test in Benchmarks:
        run_benchmark(test, ina)

    # Final readings
    end_temp = get_temperature()
    end_voltage, end_current = get_ina_readings(ina)

    print("\nFinal Sensor Readings:")
    print(f"Temperature = {end_temp:.2f} C" if end_temp is not None else "Temperature = N/A")
    print(f"Voltage = {end_voltage:.2f} V")
    print(f"Current = {end_current:.2f} mA")

    print("\nAll benchmarks complete.")

if __name__ == "__main__":
    main()