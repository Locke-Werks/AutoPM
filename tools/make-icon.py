"""Draw assets/autopm.ico.

Three solid bars hanging from one top line: a board whose columns are filled to
different levels. Drawn rather than downscaled, because a 256px render squeezed
to 16px turns to mush exactly where the icon gets looked at most, in the
taskbar. Small sizes get their own pixel-snapped geometry.

The ground is transparent. An icon that carries its own background is a tile
rather than a mark: it reads as a black box against a light taskbar, and as a
visible square against a dark one. Only the bars are drawn, so they sit on
whatever the shell puts behind them. With no container to sit inside, the mark
fills much more of the frame than it did.

    python tools/make-icon.py
"""

from PIL import Image, ImageDraw

GROUND = (0, 0, 0, 0)         # transparent: the shell supplies the ground
MARK = (176, 92, 246, 255)    # house violet

SIZES = [16, 24, 32, 48, 64, 128, 256]

# Bar heights as a fraction of the canvas, hanging from a shared top edge.
# Sized for a mark with no container: the group spans about three quarters of
# the frame rather than half.
HEIGHTS = (0.50, 0.74, 0.42)
TOP = 0.13
BAR_WIDTH = 0.205
GAP = 0.055


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

    bar = max(3, round(size * 4 / 16))
    gap = max(1, round(size * 1.6 / 16))
    group = bar * 3 + gap * 2
    x = (size - group) // 2
    top = max(1, round(size * 2 / 16))

    for fraction in (0.56, 0.80, 0.48):
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

    # The web asset keeps the violet-black tile. It sits in a grid beside the
    # other products' icons and on a README that is white, where a mark with no
    # ground would look like a mistake rather than a choice. The .ico above has
    # no ground because the shell supplies one; these are two different jobs.
    tile = Image.new("RGBA", (512, 512), (7, 5, 14, 255))
    mark = frames[-1].resize((512, 512), Image.LANCZOS)
    tile.paste(mark, (0, 0), mark)
    tile.save("assets/autopm.png")
    print("wrote assets/autopm.ico (transparent) and assets/autopm.png (tiled)")


if __name__ == "__main__":
    main()
