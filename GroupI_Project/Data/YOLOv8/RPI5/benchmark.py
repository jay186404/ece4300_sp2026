import time
import statistics
import subprocess
import numpy as np
from ultralytics import YOLO
import psutil
import matplotlib
matplotlib.use("Agg")          # headless – no display needed
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from pathlib import Path

# ── Config ────────────────────────────────────────────────────────────────────
MODEL    = "yolov8n.pt"
RUNS     = 100
WARMUP   = 10
IMG_SIZE = 640
OUT_DIR  = Path(__file__).parent   # graphs saved next to this script
# ─────────────────────────────────────────────────────────────────────────────

def get_temp():
    try:
        with open("/sys/class/thermal/thermal_zone0/temp") as f:
            return int(f.read()) / 1000.0
    except Exception:
        result = subprocess.run(["vcgencmd", "measure_temp"], capture_output=True, text=True)
        return float(result.stdout.replace("temp=", "").replace("'C", ""))

def get_power():
    freq = psutil.cpu_freq().current         # MHz
    load = psutil.cpu_percent(interval=0.1)  # %
    return round(2.5 + 2.0 * (freq / 2400) + 3.5 * (load / 100), 2)

# ── Load model ────────────────────────────────────────────────────────────────
print(f"\nLoading {MODEL}...")
model = YOLO(MODEL)
frame = np.random.randint(0, 256, (IMG_SIZE, IMG_SIZE, 3), dtype=np.uint8)

# ── Warm-up ───────────────────────────────────────────────────────────────────
print(f"Warming up ({WARMUP} frames)...")
for _ in range(WARMUP):
    model.predict(frame, verbose=False, imgsz=IMG_SIZE)

# ── Benchmark ─────────────────────────────────────────────────────────────────
print(f"Running {RUNS} frames...\n")
latencies  = []
fps_series = []
temps      = []
powers     = []
timestamps = []

t_start = time.perf_counter()

for i in range(RUNS):
    t0 = time.perf_counter()
    model.predict(frame, verbose=False, imgsz=IMG_SIZE)
    t1 = time.perf_counter()

    lat_ms = (t1 - t0) * 1000
    latencies.append(lat_ms)
    fps_series.append(1000 / lat_ms)
    temps.append(get_temp())
    powers.append(get_power())
    timestamps.append(t1 - t_start)   # seconds since benchmark start

# ── Console Results ───────────────────────────────────────────────────────────
lat = sorted(latencies)
fps = 1000 / statistics.mean(latencies)

print("=" * 40)
print("         BENCHMARK RESULTS")
print("=" * 40)
print("\n  THROUGHPUT")
print(f"    FPS          : {fps:.2f}")
print("\n  LATENCY")
print(f"    Mean         : {statistics.mean(latencies):.1f} ms")
print(f"    Min / Max    : {min(latencies):.1f} / {max(latencies):.1f} ms")
print(f"    P50          : {lat[int(0.50 * RUNS)]:.1f} ms")
print(f"    P95          : {lat[int(0.95 * RUNS)]:.1f} ms")
print(f"    P99          : {lat[int(0.99 * RUNS)]:.1f} ms")
print("\n  TEMPERATURE")
print(f"    Mean         : {statistics.mean(temps):.1f} °C")
print(f"    Max          : {max(temps):.1f} °C")
print("\n  POWER (estimated)")
print(f"    Mean         : {statistics.mean(powers):.2f} W")
print(f"    Max          : {max(powers):.2f} W")
print("\n" + "=" * 40 + "\n")

# ── Plotting ──────────────────────────────────────────────────────────────────
STYLE_LINE  = "#4A90D9"
STYLE_MEAN  = "#E74C3C"
STYLE_FILL  = "#4A90D9"
ALPHA_FILL  = 0.12
FONT_TITLE  = dict(fontsize=13, fontweight="bold")
FONT_LABEL  = dict(fontsize=10)

def add_mean_line(ax, values, label="mean"):
    m = statistics.mean(values)
    ax.axhline(m, color=STYLE_MEAN, linewidth=1.2, linestyle="--",
               label=f"{label} = {m:.2f}")
    ax.legend(fontsize=9)

def shade_area(ax, x, y):
    ax.fill_between(x, y, alpha=ALPHA_FILL, color=STYLE_FILL)

xs = timestamps   # x-axis for all plots

# ── Figure 1: Latency over time ───────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(10, 4))
ax.plot(xs, latencies, color=STYLE_LINE, linewidth=1.0)
shade_area(ax, xs, latencies)
add_mean_line(ax, latencies, "mean latency")
ax.set_title("Inference Latency over Time", **FONT_TITLE)
ax.set_xlabel("Time (s)", **FONT_LABEL)
ax.set_ylabel("Latency (ms)", **FONT_LABEL)
ax.grid(True, linestyle="--", alpha=0.4)
fig.tight_layout()
out = OUT_DIR / "benchmark_latency.png"
fig.savefig(out, dpi=150)
print(f"Saved → {out}")
plt.close(fig)

# ── Figure 2: FPS over time ───────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(10, 4))
ax.plot(xs, fps_series, color="#2ECC71", linewidth=1.0)
shade_area(ax, xs, fps_series)
ax.fill_between(xs, fps_series, alpha=ALPHA_FILL, color="#2ECC71")
add_mean_line(ax, fps_series, "mean FPS")
ax.set_title("Throughput (FPS) over Time", **FONT_TITLE)
ax.set_xlabel("Time (s)", **FONT_LABEL)
ax.set_ylabel("Frames per Second", **FONT_LABEL)
ax.grid(True, linestyle="--", alpha=0.4)
fig.tight_layout()
out = OUT_DIR / "benchmark_fps.png"
fig.savefig(out, dpi=150)
print(f"Saved → {out}")
plt.close(fig)

# ── Figure 3: Temperature over time ──────────────────────────────────────────
fig, ax = plt.subplots(figsize=(10, 4))
ax.plot(xs, temps, color="#E67E22", linewidth=1.0)
shade_area(ax, xs, temps)
ax.fill_between(xs, temps, alpha=ALPHA_FILL, color="#E67E22")
add_mean_line(ax, temps, "mean temp")
ax.set_title("CPU Temperature over Time", **FONT_TITLE)
ax.set_xlabel("Time (s)", **FONT_LABEL)
ax.set_ylabel("Temperature (°C)", **FONT_LABEL)
ax.grid(True, linestyle="--", alpha=0.4)
fig.tight_layout()
out = OUT_DIR / "benchmark_temperature.png"
fig.savefig(out, dpi=150)
print(f"Saved → {out}")
plt.close(fig)

# ── Figure 4: Power over time ─────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(10, 4))
ax.plot(xs, powers, color="#9B59B6", linewidth=1.0)
shade_area(ax, xs, powers)
ax.fill_between(xs, powers, alpha=ALPHA_FILL, color="#9B59B6")
add_mean_line(ax, powers, "mean power")
ax.set_title("Estimated Power Consumption over Time", **FONT_TITLE)
ax.set_xlabel("Time (s)", **FONT_LABEL)
ax.set_ylabel("Power (W)", **FONT_LABEL)
ax.grid(True, linestyle="--", alpha=0.4)
fig.tight_layout()
out = OUT_DIR / "benchmark_power.png"
fig.savefig(out, dpi=150)
print(f"Saved → {out}")
plt.close(fig)

# ── Figure 5: Combined 2×2 dashboard ─────────────────────────────────────────
fig = plt.figure(figsize=(14, 8))
gs  = gridspec.GridSpec(2, 2, figure=fig, hspace=0.4, wspace=0.3)

panels = [
    (gs[0, 0], latencies,  STYLE_LINE,  "Latency (ms)",       "Inference Latency",          "mean latency"),
    (gs[0, 1], fps_series, "#2ECC71",   "Frames per Second",  "Throughput (FPS)",            "mean FPS"),
    (gs[1, 0], temps,      "#E67E22",   "Temperature (°C)",   "CPU Temperature",             "mean temp"),
    (gs[1, 1], powers,     "#9B59B6",   "Power (W)",          "Estimated Power",             "mean power"),
]

for spec, data, color, ylabel, title, label in panels:
    ax = fig.add_subplot(spec)
    ax.plot(xs, data, color=color, linewidth=1.0)
    ax.fill_between(xs, data, alpha=ALPHA_FILL, color=color)
    m = statistics.mean(data)
    ax.axhline(m, color=STYLE_MEAN, linewidth=1.1, linestyle="--",
               label=f"{label} = {m:.2f}")
    ax.set_title(title, **FONT_TITLE)
    ax.set_xlabel("Time (s)", **FONT_LABEL)
    ax.set_ylabel(ylabel, **FONT_LABEL)
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(fontsize=8)

fig.suptitle("YOLOv8n Benchmark Dashboard", fontsize=15, fontweight="bold", y=1.01)
out = OUT_DIR / "benchmark_dashboard.png"
fig.savefig(out, dpi=150, bbox_inches="tight")
print(f"Saved → {out}")
plt.close(fig)

print("\nAll graphs saved.\n")
