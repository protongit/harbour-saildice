#!/usr/bin/env python3
"""Generate the Sailfish OS launcher icons for SailDice.

The artwork is white-on-transparent, uses a single isometric D6 and is
rasterised at 1024 px before being downscaled for crisp small icons.
"""

from math import cos, pi, sin, sqrt
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parent.parent
MASTER = 1024
WHITE = (255, 255, 255, 255)


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1])


def add(a, b):
    return (a[0] + b[0], a[1] + b[1])


def mul(a, factor):
    return (a[0] * factor, a[1] * factor)


def unit(vector):
    length = sqrt(vector[0] ** 2 + vector[1] ** 2)
    return (vector[0] / length, vector[1] / length)


def project(face, u, v):
    origin, edge_u, edge_v = face
    return add(origin, add(mul(edge_u, u), mul(edge_v, v)))


def pip(face, center, radius):
    _, edge_u, edge_v = face
    axis_u = mul(unit(edge_u), radius)
    axis_v = mul(unit(edge_v), radius)
    origin = project(face, center[0], center[1])
    points = []
    for step in range(40):
        angle = 2 * pi * step / 40
        offset = add(mul(axis_u, cos(angle)), mul(axis_v, sin(angle)))
        points.append(add(origin, offset))
    return points


def stroke_polygon(draw, polygon, width=38):
    for index, point in enumerate(polygon):
        draw.line([point, polygon[(index + 1) % len(polygon)]], fill=WHITE, width=width)
    radius = width // 2
    for x, y in polygon:
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=WHITE)


def main():
    image = Image.new("RGBA", (MASTER, MASTER), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image, "RGBA")

    top = (512, 150)
    right = (810, 320)
    center = (512, 490)
    left = (214, 320)
    right_bottom = (810, 650)
    bottom = (512, 820)
    left_bottom = (214, 650)

    top_face = (top, sub(right, top), sub(left, top))
    left_face = (left, sub(center, left), sub(left_bottom, left))
    right_face = (center, sub(right, center), sub(bottom, center))

    faces = [
        ([top, right, center, left], (255, 255, 255, 34)),
        ([left, center, bottom, left_bottom], (255, 255, 255, 23)),
        ([center, right, right_bottom, bottom], (255, 255, 255, 13)),
    ]
    for polygon, fill in faces:
        draw.polygon(polygon, fill=fill)
        stroke_polygon(draw, polygon)

    # One, two and three pips make the three planes immediately readable.
    draw.polygon(pip(top_face, (0.5, 0.5), 43), fill=WHITE)
    for position in ((0.30, 0.28), (0.70, 0.72)):
        draw.polygon(pip(left_face, position, 39), fill=WHITE)
    for position in ((0.28, 0.25), (0.50, 0.50), (0.72, 0.75)):
        draw.polygon(pip(right_face, position, 36), fill=WHITE)

    icons = ROOT / "icons"
    image.save(icons / "harbour-saildice-master.png")
    for size in (86, 108, 128, 172, 256):
        destination = icons / f"{size}x{size}" / "harbour-saildice.png"
        destination.parent.mkdir(parents=True, exist_ok=True)
        resized = image.resize((size, size), Image.Resampling.LANCZOS)
        resized.save(destination)
        print(destination)


if __name__ == "__main__":
    main()
