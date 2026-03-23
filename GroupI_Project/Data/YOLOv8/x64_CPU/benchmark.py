import time, platform, statistics, os, winreg


# ── CONFIGURATION ─────────────────────────────────────────
TEST_IMAGE = "https://ultralytics.com/images/bus.jpg"  # <-- change to your image path
NUM_RUNS = 50
_key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders")
SAVE_PATH = os.path.join(winreg.QueryValueEx(_key, "Desktop")[0], "benchmark_results.png")
# ──────────────────────────────────────────────────────────

def validate_image(path):
    if path.startswith("http://") or path.startswith("https://"):
        return True, "URL"
    if os.path.isfile(path):
        ext = os.path.splitext(path)[1].lower()
        if ext in [".jpg", ".jpeg", ".png", ".bmp", ".webp"]:
            return True, f"Local file ({ext})"
        else:
            return False, f"Unsupported format: {ext}"
    return False, f"File not found: {path}"

def get_gpu_metrics():
    try:
        import pynvml
        pynvml.nvmlInit()
        h = pynvml.nvmlDeviceGetHandleByIndex(0)
        return {"temp": pynvml.nvmlDeviceGetTemperature(h, pynvml.NVML_TEMPERATURE_GPU),
                "power": round(pynvml.nvmlDeviceGetPowerUsage(h)/1000.0,1),
                "util": pynvml.nvmlDeviceGetUtilizationRates(h).gpu,
                "name": pynvml.nvmlDeviceGetName(h)}
    except: return None

def get_cpu_usage():
    try:
        import psutil; return psutil.cpu_percent(interval=0.1)
    except: return None

def get_ram_usage():
    try:
        import psutil; return psutil.virtual_memory().percent
    except: return None

print(f"Python {platform.python_version()} on {platform.system()}")

valid, img_type = validate_image(TEST_IMAGE)
if not valid:
    print(f"\n  ERROR: {img_type}")
    exit(1)
print(f"  Image source : {img_type}")
print(f"  Image path   : {TEST_IMAGE}\n")

try:
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec
except ImportError:
    print("Installing matplotlib...")
    import subprocess, sys
    subprocess.run([sys.executable, "-m", "pip", "install", "matplotlib"], check=True)
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec

from ultralytics import YOLO
model = YOLO("yolo11n.pt")

print("Warming up...")
for _ in range(3): model.predict(TEST_IMAGE, verbose=False)

print(f"Running {NUM_RUNS} passes...\n")
print(f"{'Run':>4} | {'Latency':>10} | {'CPU%':>5} | {'RAM%':>5} | {'GPU':>20}")
print("-"*55)

latencies, gpu_temps, gpu_powers, cpu_usages, ram_usages, gpu = [], [], [], [], [], None

for i in range(NUM_RUNS):
    start = time.perf_counter()
    model.predict(TEST_IMAGE, verbose=False)
    latency_ms = (time.perf_counter()-start)*1000
    latencies.append(latency_ms)
    cpu, ram, gpu = get_cpu_usage(), get_ram_usage(), get_gpu_metrics()
    cpu_usages.append(cpu or 0)
    ram_usages.append(ram or 0)
    if gpu: gpu_temps.append(gpu["temp"]); gpu_powers.append(gpu["power"])
    else: gpu_temps.append(None); gpu_powers.append(None)
    if i % 5 == 0:
        g = f"{gpu['temp']}C {gpu['power']}W" if gpu else "N/A"
        print(f"{i+1:>4} | {latency_ms:>8.1f}ms | {cpu or 0:>4.1f}% | {ram or 0:>4.1f}% | {g}")

avg = statistics.mean(latencies)
runs = list(range(1, NUM_RUNS+1))

print(f"\n{'='*45}")
print(f"  Image         : {os.path.basename(TEST_IMAGE) if not TEST_IMAGE.startswith('http') else 'URL image'}")
print(f"  Avg Latency   : {avg:.2f} ms")
print(f"  Median        : {statistics.median(latencies):.2f} ms")
print(f"  Std Dev       : {statistics.stdev(latencies):.2f} ms")
print(f"  Min/Max       : {min(latencies):.2f} / {max(latencies):.2f} ms")
print(f"  Throughput    : {1000/avg:.1f} FPS")
has_gpu = any(t is not None for t in gpu_temps)
if has_gpu:
    valid_temps = [t for t in gpu_temps if t is not None]
    valid_powers = [p for p in gpu_powers if p is not None]
    print(f"  Avg GPU Temp  : {statistics.mean(valid_temps):.1f}C")
    print(f"  Max GPU Temp  : {max(valid_temps)}C")
    print(f"  Avg GPU Power : {statistics.mean(valid_powers):.1f}W")
else:
    print("  No NVIDIA GPU detected - CPU only.")
print("="*45)

fig = plt.figure(figsize=(14, 10))
fig.suptitle("Ultralytics YOLO Benchmark Results", fontsize=15, fontweight="bold")
gs = gridspec.GridSpec(2, 2, figure=fig, hspace=0.45, wspace=0.35)

ax1 = fig.add_subplot(gs[0, 0])
ax1.plot(runs, latencies, color="steelblue", linewidth=1.2)
ax1.axhline(avg, color="red", linestyle="--", linewidth=1, label=f"Avg: {avg:.1f}ms")
ax1.set_title("Latency per Run")
ax1.set_xlabel("Run #")
ax1.set_ylabel("Latency (ms)")
ax1.legend()
ax1.grid(True, alpha=0.3)

fps_per_run = [1000/l for l in latencies]
ax2 = fig.add_subplot(gs[0, 1])
ax2.plot(runs, fps_per_run, color="seagreen", linewidth=1.2)
ax2.axhline(1000/avg, color="red", linestyle="--", linewidth=1, label=f"Avg: {1000/avg:.1f} FPS")
ax2.set_title("Throughput per Run")
ax2.set_xlabel("Run #")
ax2.set_ylabel("FPS")
ax2.legend()
ax2.grid(True, alpha=0.3)

ax3 = fig.add_subplot(gs[1, 0])
ax3.plot(runs, cpu_usages, color="darkorange", linewidth=1.2, label="CPU %")
ax3.plot(runs, ram_usages, color="purple", linewidth=1.2, label="RAM %")
ax3.set_title("CPU & RAM Usage")
ax3.set_xlabel("Run #")
ax3.set_ylabel("Usage (%)")
ax3.set_ylim(0, 100)
ax3.legend()
ax3.grid(True, alpha=0.3)

ax4 = fig.add_subplot(gs[1, 1])
if has_gpu:
    ax4.plot(runs, gpu_temps, color="crimson", linewidth=1.2, label="GPU Temp (C)")
    ax4b = ax4.twinx()
    ax4b.plot(runs, gpu_powers, color="goldenrod", linewidth=1.2, linestyle="--", label="GPU Power (W)")
    ax4b.set_ylabel("Power (W)")
    ax4.set_title("GPU Temperature & Power")
    ax4.set_ylabel("Temp (C)")
    lines1, labels1 = ax4.get_legend_handles_labels()
    lines2, labels2 = ax4b.get_legend_handles_labels()
    ax4.legend(lines1+lines2, labels1+labels2, fontsize=8)
else:
    ax4.text(0.5, 0.5, "No NVIDIA GPU\nDetected", ha="center", va="center",
             fontsize=13, color="gray", transform=ax4.transAxes)
    ax4.set_title("GPU Temperature & Power")
ax4.set_xlabel("Run #")
ax4.grid(True, alpha=0.3)

plt.savefig(SAVE_PATH, dpi=150, bbox_inches="tight")
print(f"\n  Graphs saved to: {SAVE_PATH}")
plt.show()
