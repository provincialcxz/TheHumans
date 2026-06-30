#!/usr/bin/env python3
"""Generate Lyudishki app icon in all required sizes."""
import subprocess
import os
import sys
import tempfile

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Installing Pillow...")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "Pillow", "-q"])
    from PIL import Image, ImageDraw, ImageFont

SIZES = [16, 32, 64, 128, 256, 512, 1024]
BG_COLOR = (17, 20, 23)       # #111417
ACCENT = (0, 255, 65)         # #00ff41
ACCENT_DIM = (27, 158, 75)    # #1b9e4b
BORDER_COLOR = (31, 42, 31)   # #1f2a1f

PROJECT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ICON_DIR = os.path.join(PROJECT, "resources", "icons")


def draw_icon(size):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    # Background rounded rect (approximate with rectangle + slight corner)
    margin = max(1, size // 32)
    r = max(2, size // 5)
    draw.rounded_rectangle(
        [margin, margin, size - margin - 1, size - margin - 1],
        radius=r, fill=BG_COLOR, outline=BORDER_COLOR,
        width=max(1, size // 128)
    )

    # Terminal prompt ">_"
    text = ">_"
    font_size = int(size * 0.45)
    try:
        font_path = os.path.join(PROJECT, "resources", "fonts", "JetBrainsMono-Bold.ttf")
        font = ImageFont.truetype(font_path, font_size)
    except Exception:
        font = ImageFont.load_default()

    bbox = draw.textbbox((0, 0), text, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    x = (size - tw) // 2
    y = (size - th) // 2 - int(size * 0.05)
    draw.text((x, y), text, fill=ACCENT, font=font)

    # Small dot (cursor blink) after the prompt
    dot_size = max(2, size // 16)
    dx = x + tw + max(2, size // 20)
    dy = y + th - dot_size
    draw.rectangle([dx, dy, dx + dot_size, dy + dot_size], fill=ACCENT)

    return img


def main():
    os.makedirs(ICON_DIR, exist_ok=True)

    # Generate PNG sizes
    pngs = {}
    for s in SIZES:
        img = draw_icon(s)
        path = os.path.join(ICON_DIR, f"icon_{s}x{s}.png")
        img.save(path)
        pngs[s] = path
        print(f"  {s}x{s} -> {path}")

    # Save 256 as the main icon reference
    draw_icon(256).save(os.path.join(ICON_DIR, "app_icon.png"))

    # --- macOS .icns ---
    iconset = os.path.join(ICON_DIR, "Lyudishki.iconset")
    os.makedirs(iconset, exist_ok=True)
    mapping = {
        16: "icon_16x16.png",
        32: "icon_16x16@2x.png",
        32: "icon_32x32.png",
        64: "icon_32x32@2x.png",
        128: "icon_128x128.png",
        256: "icon_128x128@2x.png",
        256: "icon_256x256.png",
        512: "icon_256x256@2x.png",
        512: "icon_512x512.png",
        1024: "icon_512x512@2x.png",
    }
    # Proper mapping with all required files
    iconset_files = [
        (16, "icon_16x16.png"),
        (32, "icon_16x16@2x.png"),
        (32, "icon_32x32.png"),
        (64, "icon_32x32@2x.png"),
        (128, "icon_128x128.png"),
        (256, "icon_128x128@2x.png"),
        (256, "icon_256x256.png"),
        (512, "icon_256x256@2x.png"),
        (512, "icon_512x512.png"),
        (1024, "icon_512x512@2x.png"),
    ]
    for sz, name in iconset_files:
        src = pngs[sz]
        dst = os.path.join(iconset, name)
        img = Image.open(src)
        img.save(dst)

    icns_path = os.path.join(ICON_DIR, "Lyudishki.icns")
    subprocess.run(["iconutil", "-c", "icns", iconset, "-o", icns_path], check=True)
    print(f"\n  macOS icon -> {icns_path}")

    # --- Windows .ico ---
    ico_sizes = [16, 32, 48, 64, 128, 256]
    ico_images = []
    for s in ico_sizes:
        if s in pngs:
            ico_images.append(Image.open(pngs[s]))
        else:
            ico_images.append(draw_icon(s))
    # 48 not in standard set, generate it
    ico_images_final = []
    for s in [16, 32, 48, 64, 128, 256]:
        ico_images_final.append(draw_icon(s))

    ico_path = os.path.join(ICON_DIR, "Lyudishki.ico")
    ico_images_final[0].save(
        ico_path, format="ICO",
        sizes=[(s, s) for s in [16, 32, 48, 64, 128, 256]],
        append_images=ico_images_final[1:]
    )
    print(f"  Windows icon -> {ico_path}")

    print("\nDone!")


if __name__ == "__main__":
    main()
