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


def choice_text_rows(path, box):
    x, y, width, height = box
    # Exclude the frame; only rendered dark glyphs count, not a taller empty button.
    crop = "%dx%d+%d+%d" % (width - 12, height - 8, x - width // 2 + 6, y - height // 2 + 4)
    pixels = subprocess.check_output(["convert", path, "-crop", crop, "+repage", "rgb:-"])
    stride = (width - 12) * 3
    return sum(any(max(pixels[i:i + 3]) < 90 for i in range(row, row + stride, 3))
               for row in range(0, len(pixels), stride))


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
                event = dict(EVENT)
                if self.path.startswith("/invalid-title/"):
                    event["title"] = "Invisible\0popup"
                elif self.path.startswith("/long-choice/"):
                    event["options"] = [dict(option, text="\u6c49" * 40) for option in EVENT["options"]]
                self._send(200, chat_reply(json.dumps(event)))

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
        demo.close()
        demo = None

        failures = []
        for mode in ("invalid-title", "long-choice"):
            try:
                os.environ["FUSHENGJI_AI_BASE_URL"] = "http://127.0.0.1:%d/%s/v1" % (server.server_address[1], mode)
                requested.clear()
                release.clear()
                demo = Demo(exe, str(Path(out) / mode), ":98", ["--lang", "en", "--seed", "11"])
                demo.click((322, 427))
                assert requested.wait(5), "travel did not request the event"
                demo.dismiss_popups(random.Random(1), choose=lambda boxes: 0)
                day_one = title_pixels(demo, "day-one.png")
                probe = str(Path(demo.out) / "event.png")
                release.set()
                # A NUL title must fall back to a visible event, not an invisible locked queue.
                wait_until(event_visible)
                boxes = demo.buttons(demo.pixels(probe))
                if mode == "long-choice":
                    # Forty CJK glyphs fit the parser's 120-byte limit but need multiple lines.
                    assert all(box[3] > 36 for box in boxes), "valid long choices were clipped to one line"
                    assert choice_text_rows(probe, boxes[0]) > 18, "choice text did not render beyond one 18 px line"
                demo.click(boxes[0][:2])
                demo.click((490, 465))
                assert title_pixels(demo, "next-day.png") != day_one, "event left travel permanently locked"
                assert demo.game.poll() is None, "AI text crashed the game"
            except AssertionError as exc:
                failures.append("%s: %s" % (mode, exc))
            finally:
                release.set()
                if demo:
                    demo.close()
                    demo = None
        assert not failures, failures
    finally:
        release.set()
        if demo:
            demo.close()
        server.shutdown()
        server.server_close()
    print("PASS: Chinese render/exit; delayed AI travel lock; invalid title fallback; readable long choices")


if __name__ == "__main__":
    main()
