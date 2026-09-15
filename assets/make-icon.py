"""Draw assets/autopm.ico.

Three solid bars hanging from one top line: a board whose columns are filled to
different levels. Drawn rather than downscaled, because a 256px render squeezed
to 16px turns to mush exactly where the icon gets looked at most, in the
taskbar. Small sizes get their own pixel-snapped geometry.

    python assets/make-icon.py
"""

from PIL import Image, ImageDraw

GROUND = (7, 5, 14, 255)      # house violet-black
MARK = (176, 92, 246, 255)    # house violet

SIZES = [16, 24, 32, 48, 64, 128, 256]

# Bar heights as a fraction of the canvas, hanging from a shared top edge.
HEIGHTS = (0.40, 0.58, 0.33)
TOP = 0.21
BAR_WIDTH = 0.145
GAP = 0.0375


def draw_large(size):
    """Supersampled so the corners stay smooth at display size."""
    scale = 4
    canvas = size * scale
    image = Image.new("RGBA", (canvas, canvas), GROUND)
    draw = ImageDraw.Draw(image)

    group = 3 * BAR_WIDTH + 2 * GAP
    x = (1.0 - group) / 2.0
    radius = max(2, int(canvas * 0.022))

    for height in HEIGHTS:
        left = x * canvas
        draw.rounded_rectangle(
            [left, TOP * canvas, left + BAR_WIDTH * canvas, (TOP + height) * canvas],
            radius=radius,
            fill=MARK,
        )
        x += BAR_WIDTH + GAP

    return image.resize((size, size), Image.LANCZOS)


def draw_small(size):
    """Whole pixels only. A half-pixel edge at 16px reads as a smudge."""
    image = Image.new("RGBA", (size, size), GROUND)
    draw = ImageDraw.Draw(image)

    bar = max(2, round(size * 3 / 16))
    gap = max(1, round(size * 2 / 16))
    group = bar * 3 + gap * 2
    x = (size - group) // 2
    top = max(1, round(size * 3 / 16))

    for fraction in (0.44, 0.63, 0.38):
        height = max(2, round(size * fraction))
        draw.rectangle([x, top, x + bar - 1, top + height - 1], fill=MARK)
        x += bar + gap

    return image


def main():
    frames = [draw_small(size) if size <= 32 else draw_large(size) for size in SIZES]
    frames[-1].save(
        "assets/autopm.ico",
        format="ICO",
        sizes=[(size, size) for size in SIZES],
        append_images=frames[:-1],
    )
    frames[-1].resize((512, 512), Image.LANCZOS).save("assets/autopm.png")
    print("wrote assets/autopm.ico and assets/autopm.png")


if __name__ == "__main__":
    main()
