import subprocess, datetime, re

CIPHERS = [
    ("AES-128-CBC", "aes-128-cbc"), ("AES-192-CBC", "aes-192-cbc"), ("AES-256-CBC", "aes-256-cbc"),
    ("AES-128-CTR", "aes-128-ctr"), ("AES-192-CTR", "aes-192-ctr"), ("AES-256-CTR", "aes-256-ctr"),
    ("AES-128-GCM", "aes-128-gcm"), ("AES-192-GCM", "aes-192-gcm"), ("AES-256-GCM", "aes-256-gcm"),
]
EVENTS   = "cycles,instructions,cache-references,cache-misses,branches,branch-misses"
DURATION = "3"
SKIP     = [r"^version:", r"^built on:", r"^options:", r"^compiler:", r"^CPUINFO:", r"^OpenSSL",
            r".*unknown cipher.*", r".*evp_generic_fetch.*", r".*envelope routines.*", r".*library context.*"]

def temp():
    return subprocess.run(["vcgencmd", "measure_temp"], capture_output=True, text=True).stdout.strip().replace("temp=", "")

def clean(text):
    # Strip OpenSSL build header lines from output
    return "\n".join(l for l in text.splitlines()
                     if not any(re.search(p, l.strip(), re.IGNORECASE) for p in SKIP))

def normalize(output):
    # Extract perf elapsed time then convert all raw counters to per-second rates
    elapsed = next((float(m.group(1)) for l in output.splitlines()
                    if (m := re.search(r"([\d.]+)\s+seconds time elapsed", l))),
                   6 * int(DURATION))
    lines = []
    for l in output.splitlines():
        m = re.match(r"\s*([\d,]+)\s+(\S[\S\s]*)", l)
        if m:
            try:
                lines.append(f"  {int(m.group(1).replace(',','')) / elapsed:>18,.0f} /sec    {m.group(2).strip()}")
            except ValueError:
                if l.strip(): lines.append(f"  {l}")
        elif l.strip():
            lines.append(f"  {l}")
    return lines, elapsed

def run(label, cipher):
    print(f"\n  {'─'*54}\n  {label:<20}  temp: {temp()}\n  {'─'*54}")
    result = subprocess.run(
        ["perf", "stat", "-e", EVENTS, "openssl", "speed", "-elapsed", "-seconds", DURATION, "-evp", cipher],
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    output  = clean(result.stdout)
    norm, elapsed = normalize(output)
    # Print throughput lines then normalized perf counters
    for l in output.splitlines():
        if any(x in l for x in ["MiB/s", "MB/s", "bytes"]):
            print(f"  {l.strip()}")
    print(f"\n  perf counters (per second, elapsed: {elapsed:.2f}s):")
    for l in norm:
        if "/sec" in l: print(l)

def main():
    print("=" * 58)
    print(f"  AES Benchmark — perf stat + OpenSSL")
    print(f"  Raspberry Pi 5  |  {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"  Modes: CBC, CTR, GCM  |  Keys: 128, 192, 256-bit")
    print(f"  Window: 3s x 6 block sizes = 18s per variant")
    print("=" * 58)
    for label, cipher in CIPHERS: run(label, cipher)
    print(f"\n{'='*58}\n  Complete  |  temp: {temp()}\n{'='*58}\n")

if __name__ == "__main__":
    main()