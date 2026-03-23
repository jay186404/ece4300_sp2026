import time, platform, statistics, subprocess, json
from pathlib import Path

NUM_RUNS = 50
TEST_IMAGE = "https://ultralytics.com/images/bus.jpg"
OUT_DIR = Path.home()

def get_temp():
    """Read CPU temperature from Linux thermal sysfs."""
    try:
        with open("/sys/class/thermal/thermal_zone0/temp") as f:
            return round(int(f.read().strip()) / 1000.0, 1)
    except:
        try:
            result = subprocess.run(["vcgencmd", "measure_temp"],
                                    capture_output=True, text=True)
            return float(result.stdout.replace("temp=","").replace("'C\n",""))
        except:
            return None

def get_cpu_usage():
    try:
        import psutil; return psutil.cpu_percent(interval=0.1)
    except: return None

def get_ram_usage():
    try:
        import psutil; return psutil.virtual_memory().percent
    except: return None

def get_cpu_freq():
    try:
        import psutil; return psutil.cpu_freq().current  # MHz
    except: return None

def estimate_power():
    """Estimate power from CPU frequency and load (same method as RPi5 teammate)."""
    try:
        import psutil
        freq = psutil.cpu_freq().current   # MHz
        load = psutil.cpu_percent(interval=0.1)
        return round(2.5 + 2.0 * (freq / 2400) + 3.5 * (load / 100), 2)
    except: return None

# ── Main ───────────────────────────────────────────────────────────────────────
print(f"Python {platform.python_version()} on {platform.system()}")
print(f"Platform: Raspberry Pi 5 (Ubuntu 25.04, aarch64)")
print()

from ultralytics import YOLO
model = YOLO("yolo11n.pt")

print("Warming up...")
for _ in range(3):
    model.predict(TEST_IMAGE, verbose=False)

print(f"Running {NUM_RUNS} passes...\n")
print(f"{'Run':>4} | {'Latency':>10} | {'CPU%':>5} | {'RAM%':>5} | {'Temp':>7} | {'Power':>7}")
print("-" * 58)

latencies, cpu_usages, ram_usages, temps, powers, timestamps = [], [], [], [], [], []
t_start = time.perf_counter()

for i in range(NUM_RUNS):
    t0 = time.perf_counter()
    model.predict(TEST_IMAGE, verbose=False)
    t1 = time.perf_counter()

    latency_ms = (t1 - t0) * 1000
    latencies.append(latency_ms)
    timestamps.append(t1 - t_start)

    cpu   = get_cpu_usage()
    ram   = get_ram_usage()
    temp  = get_temp()
    power = estimate_power()

    cpu_usages.append(cpu or 0)
    ram_usages.append(ram or 0)
    temps.append(temp)
    powers.append(power)

    if i % 5 == 0:
        t_str = f"{temp:.1f}C" if temp is not None else "N/A"
        p_str = f"{power:.2f}W" if power is not None else "N/A"
        print(f"{i+1:>4} | {latency_ms:>8.1f}ms | {cpu or 0:>4.1f}% | {ram or 0:>4.1f}% | {t_str:>7} | {p_str:>7}")

# ── Summary ────────────────────────────────────────────────────────────────────
avg = statistics.mean(latencies)
lat = sorted(latencies)
valid_temps  = [t for t in temps  if t is not None]
valid_powers = [p for p in powers if p is not None]

print(f"\n{'='*50}")
print(f"  Platform      : Raspberry Pi 5 (YOLO11n)")
print(f"  Avg Latency   : {avg:.2f} ms")
print(f"  Median        : {statistics.median(latencies):.2f} ms")
print(f"  Std Dev       : {statistics.stdev(latencies):.2f} ms")
print(f"  Min/Max       : {min(latencies):.2f} / {max(latencies):.2f} ms")
print(f"  Throughput    : {1000/avg:.2f} FPS")
print(f"  P50           : {lat[int(0.50*NUM_RUNS)]:.2f} ms")
print(f"  P95           : {lat[int(0.95*NUM_RUNS)]:.2f} ms")
print(f"  P99           : {lat[int(0.99*NUM_RUNS)]:.2f} ms")
if valid_temps:
    print(f"  Avg Temp      : {statistics.mean(valid_temps):.1f} C")
    print(f"  Max Temp      : {max(valid_temps):.1f} C")
if valid_powers:
    print(f"  Avg Power     : {statistics.mean(valid_powers):.2f} W (est.)")
    print(f"  Max Power     : {max(valid_powers):.2f} W (est.)")
print(f"{'='*50}\n")

# ── Save data ──────────────────────────────────────────────────────────────────
out_data = {
    "latencies": latencies, "timestamps": timestamps,
    "cpu_usages": cpu_usages, "ram_usages": ram_usages,
    "temps": temps, "powers": powers,
}
data_path = OUT_DIR / "benchmark_pi_data.json"
with open(data_path, "w") as f:
    json.dump(out_data, f, indent=2)
print(f"Raw data saved to: {data_path}")
