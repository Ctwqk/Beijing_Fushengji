"""Check argument validation before SDL initialisation (no display required)."""
import subprocess
import sys

exe = sys.argv[1]
failures = []
for option, values in (
    ("--days", ["0", "-1", "41", "300", "abc", "2days", "99999999999999999999"]),
    ("--seed", ["-1", "abc", "1seed", "4294967296"]),
):
    for value in values:
        # --help exits before opening a window if validation wrongly accepts the value.
        result = subprocess.run([exe, option, value, "--help"], capture_output=True, text=True, timeout=5)
        if result.returncode != 2:
            failures.append((option, value, result.returncode, result.stderr))

for args in (["--help"], ["--days", "1", "--help"], ["--days", "40", "--help"],
             ["--seed", "0", "--help"], ["--seed", "4294967295", "--help"]):
    result = subprocess.run([exe] + args, capture_output=True, text=True, timeout=5)
    assert result.returncode == 0, (args, result.stderr)
    assert "Usage:" in result.stdout
assert not failures, failures
print("PASS: invalid arguments rejected; supported day and seed boundaries accepted")
