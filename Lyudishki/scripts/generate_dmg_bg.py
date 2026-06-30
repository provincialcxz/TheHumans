#!/usr/bin/env python3
"""Generate Mr. Robot styled DMG background image."""
from PIL import Image, ImageDraw, ImageFont
import os

PROJECT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONT_PATH = os.path.join(PROJECT, "resources", "fonts", "JetBrainsMono-Bold.ttf")
FONT_REG = os.path.join(PROJECT, "resources", "fonts", "JetBrainsMono-Regular.ttf")
OUT = os.path.join(PROJECT, "resources", "icons", "dmg_background.png")
OUT_2X = os.path.join(PROJECT, "resources", "icons", "dmg_background@2x.png")

W, H = 660, 400

BG = (10, 10, 10)          # #0a0a0a
PANEL = (17, 20, 23)       # #111417
GREEN = (0, 255, 65)       # #00ff41
DIM = (27, 158, 75)        # #1b9e4b
GRAY = (125, 140, 125)     # #7d8c7d
BORDER = (31, 42, 31)      # #1f2a1f
RED = (255, 59, 48)        # #ff3b30


def draw_bg(scale=1):
    w, h = W * scale, H * scale
    img = Image.new("RGB", (w, h), BG)
    draw = ImageDraw.Draw(img)

    def s(v):
        return int(v * scale)

    def font(size, bold=False):
        try:
            return ImageFont.truetype(FONT_PATH if bold else FONT_REG, s(size))
        except Exception:
            return ImageFont.load_default()

    # Top bar
    draw.rectangle([0, 0, w, s(36)], fill=PANEL)
    draw.line([0, s(36), w, s(36)], fill=BORDER, width=s(1))

    # Title in bar
    f_title = font(13, bold=True)
    draw.text((s(20), s(10)), ">_ Людишки", fill=DIM, font=f_title)

    # Version tag
    f_ver = font(10)
    draw.text((w - s(120), s(12)), "v1.0.0", fill=GRAY, font=f_ver)

    # Scanline effect (subtle horizontal lines)
    for y in range(0, h, s(3)):
        draw.line([0, y, w, y], fill=(8, 8, 8), width=1)

    # Terminal text decorations (top area)
    f_term = font(9)
    lines = [
        ("$ installing lyudishki...", DIM),
        ("  [████████████████████] 100%", GREEN),
        ("  drag app to /Applications", GRAY),
    ]
    ty = s(52)
    for text, color in lines:
        draw.text((s(24), ty), text, fill=color, font=f_term)
        ty += s(16)

    # Arrow zone — between icon positions
    # Icons will be at roughly: App at x=200, Applications at x=460 (centered in 660)
    arrow_y = s(230)
    arrow_x_start = s(260)
    arrow_x_end = s(400)

    # Draw arrow shaft
    shaft_w = s(3)
    draw.rectangle([arrow_x_start, arrow_y - shaft_w//2,
                     arrow_x_end - s(12), arrow_y + shaft_w//2], fill=DIM)

    # Arrowhead
    head_size = s(14)
    draw.polygon([
        (arrow_x_end, arrow_y),
        (arrow_x_end - head_size, arrow_y - head_size//2),
        (arrow_x_end - head_size, arrow_y + head_size//2),
    ], fill=GREEN)

    # Dotted circle hints where icons go
    def draw_circle_hint(cx, cy, r, color):
        for angle in range(0, 360, 8):
            import math
            x = cx + int(r * math.cos(math.radians(angle)))
            y = cy + int(r * math.sin(math.radians(angle)))
            draw.rectangle([x-s(1), y-s(1), x+s(1), y+s(1)], fill=color)

    draw_circle_hint(s(200), s(220), s(40), BORDER)
    draw_circle_hint(s(460), s(220), s(40), BORDER)

    # Bottom bar
    draw.rectangle([0, h - s(32), w, h], fill=PANEL)
    draw.line([0, h - s(32), w, h - s(32)], fill=BORDER, width=s(1))

    # Bottom text
    f_bottom = font(9)
    draw.text((s(24), h - s(24)), "$ echo \"enjoy hacking\"", fill=GRAY, font=f_bottom)

    # Red cursor blinking dot
    draw.rectangle([s(260), h - s(24), s(268), h - s(16)], fill=RED)

    return img


if __name__ == "__main__":
    img1x = draw_bg(1)
    img1x.save(OUT)
    print(f"1x: {OUT}")

    img2x = draw_bg(2)
    img2x.save(OUT_2X)
    print(f"2x: {OUT_2X}")
    print("Done!")
