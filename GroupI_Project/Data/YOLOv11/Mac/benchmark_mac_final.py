import time, platform, statistics, subprocess, threading, re, json
from pathlib import Path

NUM_RUNS = 50
TEST_IMAGE = "https://ultralytics.com/images/bus.jpg"
OUT_DIR = Path.home() / "Desktop"

_pm_data = {"power": None}
_pm_lock = threading.Lock()
_pm_stop = threading.Event()

def _powermetrics_thread():
    """
    Parse powermetrics text output for Mac14,2 (MacBook Air M2).
    This model does NOT expose CPU die temperature via powermetrics.
    Power lines look like:
        CPU Power: 173 mW
        GPU Power: 89 mW
        ANE Power: 0 mW
        Combined Power (CPU + GPU + ANE): 261 mW
    We collect Combined Power as it reflects the full SoC load.
    """
    try:
        proc = subprocess.Popen(
            ["sudo", "powermetrics", "--samplers", "cpu_power",
             "-i", "1000", "-n", "0"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True
        )
        for line in proc.stdout:
            if _pm_stop.is_set():
                proc.terminate()
                break
            # "Combined Power (CPU + GPU + ANE): 261 mW"
            m = re.search(r'Combined Power[^:]*:\s*([\d.]+)\s*mW', line)
            if m:
                with _pm_lock:
                    _pm_data["power"] = round(float(m.group(1)) / 1000.0, 3)
    except Exception:
        pass

def get_power():
    with _pm_lock:
        return _pm_data["power"]

def get_cpu_usage():
    try:
        import psutil; return psutil.cpu_percent(interval=0.1)
    except: return None

def get_ram_usage():
    try:
        import psutil; return psutil.virtual_memory().percent
    except: return None

# ── Main ───────────────────────────────────────────────────────────────────────
print(f"Python {platform.python_version()} on {platform.system()}")
print(f"Machine: Mac14,2 (MacBook Air M2)")
print()
print("Note: MacBook Air M2 does not expose CPU temperature via powermetrics.")
print("      Combined SoC power (CPU + GPU + ANE) will be collected instead.")
print()
print("Starting powermetrics (requires sudo)...")

pm_thread = threading.Thread(target=_powermetrics_thread, daemon=True)
pm_thread.start()
time.sleep(3)

p = get_power()
if p is not None:
    print(f"powermetrics active — initial power reading: {p:.3f} W")
else:
    print("WARNING: No power reading yet — will keep collecting during run.")
print()

from ultralytics import YOLO
model = YOLO("yolo11n.pt")

print("Warming up...")
for _ in range(3):
    model.predict(TEST_IMAGE, verbose=False)

print(f"Running {NUM_RUNS} passes...\n")
print(f"{'Run':>4} | {'Latency':>10} | {'CPU%':>5} | {'RAM%':>5} | {'SoC Power':>10}")
print("-" * 55)

latencies, cpu_usages, ram_usages, powers, timestamps = [], [], [], [], []
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
    power = get_power()

    cpu_usages.append(cpu or 0)
    ram_usages.append(ram or 0)
    powers.append(power)

    if i % 5 == 0:
        p_str = f"{power:.3f} W" if power is not None else "N/A"
        print(f"{i+1:>4} | {latency_ms:>8.1f}ms | {cpu or 0:>4.1f}% | {ram or 0:>4.1f}% | {p_str:>10}")

_pm_stop.set()

# ── Summary ────────────────────────────────────────────────────────────────────
avg = statistics.mean(latencies)
valid_powers = [p for p in powers if p is not None]

print(f"\n{'='*50}")
print(f"  Platform        : MacBook Air (Apple Silicon M2)")
print(f"  Avg Latency     : {avg:.2f} ms")
print(f"  Median          : {statistics.median(latencies):.2f} ms")
print(f"  Std Dev         : {statistics.stdev(latencies):.2f} ms")
print(f"  Min/Max         : {min(latencies):.2f} / {max(latencies):.2f} ms")
print(f"  Throughput      : {1000/avg:.1f} FPS")
print(f"  CPU Temp        : N/A (not exposed on Mac14,2)")
if valid_powers:
    print(f"  Avg SoC Power   : {statistics.mean(valid_powers):.3f} W")
    print(f"  Max SoC Power   : {max(valid_powers):.3f} W")
    print(f"  (Combined CPU + GPU + ANE)")
else:
    print(f"  SoC Power       : Not collected")
print(f"{'='*50}\n")

# ── Save data ──────────────────────────────────────────────────────────────────
out_data = {
    "latencies": latencies,
    "timestamps": timestamps,
    "cpu_usages": cpu_usages,
    "ram_usages": ram_usages,
    "temps": [None] * NUM_RUNS,   # not available on this model
    "powers": powers,
}
data_path = OUT_DIR / "benchmark_mac_data.json"
with open(data_path, "w") as f:
    json.dump(out_data, f, indent=2)
print(f"Raw data saved to: {data_path}")
print("Run generate_mac_graphs.py next to produce the graphs.")
