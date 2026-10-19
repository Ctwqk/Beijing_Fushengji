"""Exercise the real SDL/ImGui window, including travel during a delayed AI event.

Run in docker/Dockerfile's image: python3 tests/test_gui.py build/Beijing_Fushengji build/gui-test
Uses Xvfb, Mesa, xdotool and ImageMagick; never contacts an external AI service.
"""
import json
import os
from pathlib import Path
import random
import subprocess
import sys
import threading
import time
from http.server import ThreadingHTTPServer

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from capture_demo import Demo  # noqa: E402
from mock_openai_server import EVENT, Handler, chat_reply  # noqa: E402


def wait_until(predicate, timeout=5):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(0.1)
    raise AssertionError("timed out waiting for the GUI")


def title_pixels(demo, name):
    path = str(Path(demo.out) / name)
    demo.grab(path)
    return subprocess.check_output(["convert", path, "-crop", "760x28+0+0", "+repage", "rgb:-"])


def main():
    exe, out = sys.argv[1:3]
    # Ensure a developer's shell cannot change the test's event source.
    for name in tuple(os.environ):
        if name.startswith("FUSHENGJI_"):
            del os.environ[name]

    # Also prove the shipped font can render the Chinese GUI from a different cwd.
    zh = Demo(exe, str(Path(out) / "zh"), ":98", ["--lang", "zh", "--offline", "--seed", "11"])
    try:
        assert zh.game.poll() is None, "Chinese game exited during startup"
        zh.frame("main-zh")
        zh.game.terminate()
        assert zh.game.wait(timeout=5) == 0, "Chinese game did not close cleanly"
    finally:
        zh.close()

    requested, release = threading.Event(), threading.Event()

    class DelayedHandler(Handler):
        def do_POST(self):
            self.rfile.read(int(self.headers.get("Content-Length", 0)))
            requested.set()
            if release.wait(20):
                self._send(200, chat_reply(json.dumps(EVENT)))

    server = ThreadingHTTPServer(("127.0.0.1", 0), DelayedHandler)
    server.daemon_threads = True
    threading.Thread(target=server.serve_forever, daemon=True).start()
    os.environ["FUSHENGJI_AI_BASE_URL"] = "http://127.0.0.1:%d/v1" % server.server_address[1]
    os.environ["FUSHENGJI_AI_MODEL"] = "mock-model"
    os.environ["FUSHENGJI_AI_TIMEOUT"] = "30"
    demo = None
    try:
        demo = Demo(exe, str(Path(out) / "en"), ":98", ["--lang", "en", "--seed", "11"])
        assert demo.game.poll() is None, "English game exited during startup"
        demo.click((322, 427))  # seed 11 generates a life event on the first move
        assert requested.wait(5), "travel did not request the expected event"
        demo.dismiss_popups(random.Random(1), choose=lambda boxes: 0)  # dismiss that day's news
        before = title_pixels(demo, "waiting.png")
        demo.click((490, 465))
        after = title_pixels(demo, "attempted-travel.png")
        assert before == after, "day advanced while the AI event was still pending"

        release.set()
        probe = str(Path(demo.out) / "event.png")

        def event_visible():
            demo.grab(probe)
            return len(demo.buttons(demo.pixels(probe))) == 3

        wait_until(event_visible)
        demo.frame("ai-event")
        boxes = demo.buttons(demo.pixels(probe))
        demo.click(boxes[0][:2])
        demo.click((490, 465))
        assert title_pixels(demo, "next-day.png") != before, "travel stayed locked after the event was resolved"
        demo.game.terminate()
        assert demo.game.wait(timeout=5) == 0, "English game did not close cleanly"
    finally:
        release.set()
        if demo:
            demo.close()
        server.shutdown()
        server.server_close()
    print("PASS: Chinese render/exit; delayed AI event blocks travel, then travel resumes")


if __name__ == "__main__":
    main()
