"""Starts the mock OpenAI server on a free port and runs the given test executable against it."""
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mock_openai_server  # noqa: E402


def main():
    if len(sys.argv) < 2:
        print("usage: run_with_mock_server.py <test-executable> [args...]")
        return 2
    server = mock_openai_server.start(0)
    env = dict(os.environ)
    env["FSJ_MOCK_URL"] = "http://127.0.0.1:%d" % server.server_address[1]
    # The tests must never pick up a real key from the developer's shell.
    for name in ("FUSHENGJI_AI_API_KEY", "FUSHENGJI_AI_BASE_URL", "FUSHENGJI_AI_MODEL", "FUSHENGJI_OFFLINE"):
        env.pop(name, None)
    try:
        return subprocess.call(sys.argv[1:], env=env)
    finally:
        server.shutdown()


if __name__ == "__main__":
    sys.exit(main())
