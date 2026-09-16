"""Assert that an .ico carries every size the Windows shell asks for.

A single 256 pixel image left for the shell to downscale turns to mush at 16,
which is the size the icon is looked at most: the notification area, the
taskbar, Alt-Tab. make-icon.py draws the small sizes separately for that
reason, and this is the check that the file it produced still has them.

Reads the icon directory only, so it needs nothing but the standard library
and cannot disagree with Pillow about how an image decodes.

    python tools/check-icon.py assets/autopm.ico
"""

import struct
import sys

REQUIRED = (16, 24, 32, 48, 64, 128, 256)


def entries(path):
    with open(path, "rb") as handle:
        data = handle.read()

    if len(data) < 6:
        raise SystemExit("%s is too short to be an icon" % path)
    reserved, kind, count = struct.unpack_from("<HHH", data, 0)
    if reserved != 0 or kind != 1:
        raise SystemExit("%s is not an icon file (reserved=%d type=%d)" % (path, reserved, kind))

    found = []
    for index in range(count):
        offset = 6 + index * 16
        if offset + 16 > len(data):
            raise SystemExit("%s: directory entry %d runs past the end" % (path, index))
        width, height, _colours, _pad, _planes, bits, size, at = struct.unpack_from(
            "<BBBBHHII", data, offset
        )
        # 0 means 256 in an icon directory: the field is one byte.
        found.append(
            {
                "width": width or 256,
                "height": height or 256,
                "bits": bits,
                "bytes": size,
                "offset": at,
                "png": data[at : at + 8] == b"\x89PNG\r\n\x1a\n",
            }
        )
        if at + size > len(data):
            raise SystemExit("%s: image %d runs past the end" % (path, index))
    return found


def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: check-icon.py <file.ico>")
    path = sys.argv[1]
    found = entries(path)

    for entry in found:
        print(
            "  %4d x %-4d %3d bpp  %6d bytes%s"
            % (
                entry["width"],
                entry["height"],
                entry["bits"],
                entry["bytes"],
                "  png" if entry["png"] else "",
            )
        )

    problems = []
    present = {entry["width"] for entry in found if entry["width"] == entry["height"]}
    missing = [size for size in REQUIRED if size not in present]
    if missing:
        problems.append("missing %s" % ", ".join("%dpx" % size for size in missing))

    for entry in found:
        if entry["width"] != entry["height"]:
            problems.append("%dx%d is not square" % (entry["width"], entry["height"]))
        # A PNG-compressed image carries its own depth, so the directory's
        # advisory bit count is only worth checking on the BMP ones.
        if not entry["png"] and entry["bits"] not in (0, 32):
            problems.append("%dpx is %d bpp, not 32" % (entry["width"], entry["bits"]))

    if problems:
        for problem in problems:
            print("FAIL: %s" % problem)
        raise SystemExit(1)

    print("%s: %d images, every required size present" % (path, len(found)))


if __name__ == "__main__":
    main()
