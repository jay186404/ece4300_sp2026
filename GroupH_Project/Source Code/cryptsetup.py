import subprocess, datetime, re

CIPHERS = [
    ("AES-128-CBC", "aes-cbc", "128"), ("AES-192-CBC", "aes-cbc", "192"), ("AES-256-CBC", "aes-cbc", "256"),
    ("AES-128-CTR", "aes-ctr", "128"), ("AES-192-CTR", "aes-ctr", "192"), ("AES-256-CTR", "aes-ctr", "256"),
]
EVENTS = "cycles,instructions,cache-references,cache-misses,branches,branch-misses"

def temp():
    return subprocess.run(["vcgencmd", "measure_temp"], capture_output=True, text=True).stdout.strip().replace("temp=", "")

def check_permissions():
    # Warn if perf is restricted to userspace counters only
    try:
        level = int(open("/proc/sys/kernel/perf_event_paranoid").read().strip())
        if level > 0:
            print(f"  WARNING: perf_event_paranoid={level} — kernel cycles will be missed")
            print("  Fix: sudo sh -c 'echo -1 > /proc/sys/kernel/perf_event_paranoid'\n")
        else:
            print(f"  perf permissions OK (paranoid={level})\n")
    except Exception:
        print("  WARNING: could not read perf_event_paranoid\n")

def is_counter(line):
    return bool(re.match(r"\s+[\d,]+\s+\S", line))

def normalize_line(line, elapsed):
    # Convert a raw perf counter line to a per-second rate
    m = re.match(r"\s*([\d,]+)\s+(\S+)(.*)", line)
    if not m: return None
    try:
        per_sec = int(m.group(1).replace(",", "")) / elapsed
        base    = f"            {per_sec:>13,.0f} /sec    {m.group(2).strip()}"
        note    = m.group(3).strip()
        return f"{base:<54}  {note}" if note.startswith("#") else base
    except ValueError:
        return None

def run(label, cipher, keysize):
    print(f"\n  {'─'*54}\n  {label:<20}  temp: {temp()}\n  {'─'*54}", flush=True)
    proc = subprocess.Popen(
        ["perf", "stat", "-e", EVENTS, "cryptsetup", "benchmark", "--cipher", cipher, "--key-size", keysize],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)
    # Stream throughput lines live as cryptsetup produces them
    for line in proc.stdout:
        if not line.startswith("#") and ("MiB/s" in line or "GiB/s" in line):
            print(f"  {line.rstrip()}", flush=True)
    stderr = proc.stderr.readlines()
    proc.wait()
    elapsed = next((float(m.group(1)) for l in stderr
                    if (m := re.search(r"([\d.]+)\s+seconds time elapsed", l))), 2.0)
    # Print normalized counters and elapsed time
    print(f"  perf counters (per second, elapsed: {elapsed:.2f}s):")
    for line in stderr:
        if is_counter(line) and "seconds time elapsed" not in line:
            out = normalize_line(line, elapsed)
            if out: print(out)
    for line in stderr:
        if "seconds time elapsed" in line:
            print(f"           {line.rstrip()}")

def main():
    check_permissions()
    print("=" * 58)
    print(f"  AES Benchmark — cryptsetup benchmark")
    print(f"  Raspberry Pi 5  |  {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"  Modes: CBC, CTR  |  Keys: 128, 192, 256-bit")
    print(f"  GCM: not supported by cryptsetup")
    print(f"  Counters: normalized to per-second rates")
    print("=" * 58)
    for label, cipher, keysize in CIPHERS: run(label, cipher, keysize)
    print(f"\n{'='*58}\n  Complete  |  temp: {temp()}\n{'='*58}\n")

if __name__ == "__main__":
    main()