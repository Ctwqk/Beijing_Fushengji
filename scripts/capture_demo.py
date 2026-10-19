#!/usr/bin/env python3
"""Plays the real game under Xvfb and records screenshots plus the frames of the demo GIF.

It is meant to run inside the Docker image from docker/Dockerfile (see
scripts/capture_demo.sh): it needs Xvfb, xdotool and ImageMagick, drives the game
with synthetic mouse clicks and decides what to click by looking at screenshots.
The game itself runs unmodified, offline and with a fixed seed.

  capture_demo.py --exe build/Beijing_Fushengji --out /tmp/frames --scenario tour-zh --seed 11 --days 7
"""
import argparse
import os
import random
import subprocess
import sys
import time

W, H = 760, 660
BG = (226, 218, 202)                         # main window background
BUTTON_COLORS = {(192, 182, 170), (205, 195, 180), (180, 170, 155)}  # normal / hovered / pressed

# Geometry of the 760x660 main window (Chinese and English layouts are identical).
MARKET_FIRST_ROW = (150, 102)
ROW_HEIGHT = 22
BUY_BUTTON = (352, 58)
SELL_BUTTON = (352, 104)
INVENTORY_FIRST_ROW = (500, 102)
LOCATION_COLUMNS = (322, 490, 661)
LOCATION_ROWS = (427, 465, 503)
BANK, HOSPITAL, POST = (78, 604), (228, 604), (378, 604)


def sh(cmd, **kw):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True, **kw).stdout


class Demo:
    def __init__(self, exe, out, display, args):
        self.out = out
        self.frames = []
        os.makedirs(out, exist_ok=True)
        os.environ["DISPLAY"] = display
        self.xvfb = subprocess.Popen(["Xvfb", display, "-screen", "0", f"{W}x{H}x24"],
                                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(1.0)
        env = dict(os.environ, LIBGL_ALWAYS_SOFTWARE="1")
        exe = os.path.abspath(exe)
        self.game = subprocess.Popen([exe] + args, env=env, cwd=os.path.dirname(exe),
                                     stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        time.sleep(3.0)

    def close(self):
        self.game.terminate()
        self.xvfb.terminate()

    # --- input -----------------------------------------------------------------
    def click(self, pos, wait=0.45):
        sh(f"xdotool mousemove {pos[0]} {pos[1]}")
        time.sleep(0.15)
        sh("xdotool click 1")
        time.sleep(wait)

    # --- vision ----------------------------------------------------------------
    def grab(self, path):
        subprocess.run(["import", "-window", "root", path], check=True)

    def frame(self, name=None):
        """Saves a screenshot as the next GIF frame (and under `name` when given)."""
        path = os.path.join(self.out, f"frame{len(self.frames):03d}.png")
        self.grab(path)
        self.frames.append(path)
        if name:
            self.grab(os.path.join(self.out, name + ".png"))
        return path

    @staticmethod
    def pixels(path, step=2):
        """Returns {(x, y): (r, g, b)} sampled every `step` pixels (nearest neighbour, exact colours)."""
        pct = 100 // step
        text = sh(f"convert {path} -sample {pct}% txt:-")
        px = {}
        for line in text.splitlines()[1:]:
            coord, rest = line.split(":", 1)
            x, y = coord.split(",")
            r, g, b = rest.split("(")[1].split(")")[0].split(",")[:3]
            px[(int(x) * step, int(y) * step)] = (int(r), int(g), int(b))
        return px

    def modal_open(self, px):
        return px.get((746, 652)) != BG

    def buttons(self, px, step=2):
        """Bounding boxes (centre x, centre y, w, h) of button-coloured regions."""
        cells = {p for p, c in px.items() if c in BUTTON_COLORS}
        seen, boxes = set(), []
        for start in sorted(cells):
            if start in seen:
                continue
            stack, comp = [start], []
            seen.add(start)
            while stack:
                x, y = stack.pop()
                comp.append((x, y))
                for d in ((step, 0), (-step, 0), (0, step), (0, -step)):
                    n = (x + d[0], y + d[1])
                    if n in cells and n not in seen:
                        seen.add(n)
                        stack.append(n)
            xs, ys = [p[0] for p in comp], [p[1] for p in comp]
            w, h = max(xs) - min(xs) + step, max(ys) - min(ys) + step
            if w >= 60 and h >= 24:
                boxes.append(((min(xs) + max(xs)) // 2, (min(ys) + max(ys)) // 2, w, h))
        return sorted(boxes, key=lambda b: (b[1], b[0]))

    def rows_with_text(self, path, x0, x1, y0, count):
        """Number of consecutive table rows (22px apart) that contain dark text."""
        n = 0
        for i in range(count):
            y = y0 + ROW_HEIGHT * i
            out = sh(f"convert {path} -crop {x1 - x0}x{ROW_HEIGHT - 6}+{x0}+{y - 8} +repage -colorspace Gray -format %[min] info:")
            try:
                dark = float(out) < 0.45 * 65535 if float(out) > 1.0 else float(out) < 0.45
            except ValueError:
                dark = False
            if not dark:
                break
            n += 1
        return n

    def dismiss_popups(self, rng, choose=None, name_prefix=None, limit=12):
        """Clicks through modal popups; `choose` picks a button index (default random)."""
        shown = 0
        for _ in range(limit):
            time.sleep(0.25)
            path = os.path.join(self.out, "probe.png")
            self.grab(path)
            px = self.pixels(path)
            if not self.modal_open(px):
                return shown
            boxes = self.buttons(px)
            if not boxes:
                return shown
            name = f"{name_prefix}{shown + 1}" if name_prefix else None
            self.frame(name)
            idx = choose(boxes) if choose else rng.randrange(len(boxes))
            self.click(boxes[idx][:2])
            shown += 1
        return shown


    def buy_dialog(self, quantity, name=None):
        """Handles the quantity dialog after pressing Buy: types `quantity` and confirms."""
        time.sleep(0.4)
        path = os.path.join(self.out, "probe.png")
        self.grab(path)
        px = self.pixels(path)
        if not self.modal_open(px):
            return False
        boxes = self.buttons(px)
        if len(boxes) < 2:                       # an error message instead of the dialog
            self.frame()
            if boxes:
                self.click(boxes[0][:2])
            return False
        confirm = boxes[0]
        self.frame(name)
        self.click((confirm[0] - 30, confirm[1] - 63), wait=0.3)   # the quantity field
        sh("xdotool key ctrl+a")
        sh(f"xdotool type {quantity}")
        time.sleep(0.4)
        self.frame()
        self.click(confirm[:2])
        return True


def scenario_tour(demo, rng, days):
    """A few days of trading in the selected language, visiting every kind of popup."""
    demo.frame("main")
    holding = False
    for day in range(days):
        # sell what we carry every other day (select the first inventory line, then Sell)
        if holding and day % 2 == 1:
            demo.click(INVENTORY_FIRST_ROW)
            demo.click(SELL_BUTTON)
            demo.dismiss_popups(rng, choose=lambda b: 0)
            holding = False
        # buy the last listed good (the cheap ones are listed last)
        path = os.path.join(demo.out, "probe.png")
        demo.grab(path)
        rows = demo.rows_with_text(path, 16, 200, MARKET_FIRST_ROW[1], 8)
        if rows and not holding:
            demo.click((MARKET_FIRST_ROW[0], MARKET_FIRST_ROW[1] + ROW_HEIGHT * (rows - 1)))
            demo.frame("select" if day == 0 else None)
            demo.click(BUY_BUTTON)
            holding = demo.buy_dialog(4, name="trade" if day == 0 else None)
        # visit the town services once each
        if day == 1:
            demo.click(BANK)
            demo.dismiss_popups(rng, choose=lambda b: 0, name_prefix="bank-")
        if day == 2:
            demo.click(POST)
            demo.dismiss_popups(rng, choose=lambda b: 0, name_prefix="post-")
        if day == 3:
            demo.click(HOSPITAL)
            demo.dismiss_popups(rng, choose=lambda b: 0, name_prefix="hospital-")
        # travel
        demo.click((rng.choice(LOCATION_COLUMNS), rng.choice(LOCATION_ROWS)))
        demo.dismiss_popups(rng, name_prefix="popup-day%d-" % (day + 1))
        if demo.game.poll() is not None:          # the ending popup quits the game
            break
        demo.frame("main-day%d" % (day + 1))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--exe", required=True, help="path of the Beijing_Fushengji executable")
    ap.add_argument("--out", required=True, help="directory for the screenshots and GIF frames")
    ap.add_argument("--scenario", choices=["tour-zh", "tour-en", "language"], required=True)
    ap.add_argument("--seed", type=int, default=7, help="game seed (also seeds the click choices)")
    ap.add_argument("--days", type=int, default=8, help="number of days the script plays")
    ap.add_argument("--game-days", type=int, default=40, help="length of the game; use a small value to reach the ending")
    ap.add_argument("--display", default=":99")
    a = ap.parse_args()

    rng = random.Random(a.seed)
    game_args = ["--seed", str(a.seed), "--offline", "--days", str(a.game_days)]
    if a.scenario != "language":
        game_args += ["--lang", "zh" if a.scenario.endswith("zh") else "en"]
    demo = Demo(a.exe, a.out, a.display, game_args)
    try:
        if a.scenario == "language":
            demo.frame("language")
        else:
            scenario_tour(demo, rng, a.days)
    finally:
        demo.close()
    print("\n".join(demo.frames))


if __name__ == "__main__":
    sys.exit(main())
