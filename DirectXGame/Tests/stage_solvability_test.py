"""Search playable solutions using the constants and update order from Player.cpp."""

from __future__ import annotations

import csv
import itertools
import math
import sys
from pathlib import Path


DT = 1.0 / 60.0
FRICTION = 0.992
STOP_SPEED = 0.09
HIT_RADIUS_SQ = 0.26**2
HALF_LENGTH = 0.48


def cpp_round(value: float) -> int:
    return math.floor(value + 0.5) if value >= 0 else math.ceil(value - 0.5)


def segment_point_distance_sq(p, a, b):
    abx, abz = b[0] - a[0], b[1] - a[1]
    apx, apz = p[0] - a[0], p[1] - a[1]
    length_sq = abx * abx + abz * abz
    t = 0.0 if length_sq <= 0.0001 else max(0.0, min(1.0, (apx * abx + apz * abz) / length_sq))
    x, z = a[0] + abx * t, a[1] + abz * t
    return (p[0] - x) ** 2 + (p[1] - z) ** 2


def cross(a, b, c):
    return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])


def intersects(a, b, c, d):
    eps = 0.0001
    if max(min(a[0], b[0]), min(c[0], d[0])) > min(max(a[0], b[0]), max(c[0], d[0])) + eps:
        return False
    if max(min(a[1], b[1]), min(c[1], d[1])) > min(max(a[1], b[1]), max(c[1], d[1])) + eps:
        return False
    return cross(a, b, c) * cross(a, b, d) <= eps and cross(c, d, a) * cross(c, d, b) <= eps


def segment_distance_sq(a, b, c, d):
    if intersects(a, b, c, d):
        return 0.0
    return min(
        segment_point_distance_sq(a, c, d), segment_point_distance_sq(b, c, d),
        segment_point_distance_sq(c, a, b), segment_point_distance_sq(d, a, b),
    )


class Stage:
    def __init__(self, path: Path):
        self.path = path
        with path.open(newline="") as f:
            self.rows = [[int(v) for v in row] for row in csv.reader(f)]
        self.h, self.w = len(self.rows), len(self.rows[0])
        self.origin = (-(self.w - 1) / 2.0, -(self.h - 1) / 2.0)
        self.walls = {(x, z) for z, row in enumerate(self.rows) for x, v in enumerate(row) if v == 1}
        self.places = [(x, z) for z, row in enumerate(self.rows) for x, v in enumerate(row) if v in (0, 4, 5, 6, 7)]
        self.marked = [(x, z) for z, row in enumerate(self.rows) for x, v in enumerate(row) if v == 4]
        self.start = next((x, z) for z, row in enumerate(self.rows) for x, v in enumerate(row) if v == 2)
        self.goal = next((x, z) for z, row in enumerate(self.rows) for x, v in enumerate(row) if v == 3)

    def world(self, grid):
        return self.origin[0] + grid[0], self.origin[1] + grid[1]

    def grid(self, pos):
        return cpp_round(pos[0] - self.origin[0]), cpp_round(pos[1] - self.origin[1])


def simulate(stage: Stage, start_x: int, angle_deg: float, speed: float, gimmicks):
    pos = list(stage.world((start_x, stage.start[1])))
    angle = math.radians(angle_deg)
    vel = [math.sin(angle) * speed, -math.cos(angle) * speed]
    last_reflector = None
    last_panel = None
    for frame in range(2000):
        previous = tuple(pos)
        previous_grid = stage.grid(previous)
        pos[0] += vel[0] * DT
        pos[1] += vel[1] * DT
        vel[0] *= FRICTION
        vel[1] *= FRICTION
        current_grid = stage.grid(pos)
        if not (0 <= current_grid[0] < stage.w and 0 <= current_grid[1] < stage.h) or current_grid in stage.walls:
            pos[:] = previous
            if current_grid[0] != previous_grid[0]:
                vel[0] *= -1
            if current_grid[1] != previous_grid[1]:
                vel[1] *= -1
            current_grid = stage.grid(pos)

        nearest = None
        for grid, kind in gimmicks.items():
            if kind not in ("/", "\\"):
                continue
            cx, cz = stage.world(grid)
            direction = (math.sqrt(0.5), math.sqrt(0.5) if kind == "\\" else -math.sqrt(0.5))
            a = (cx - direction[0] * HALF_LENGTH, cz - direction[1] * HALF_LENGTH)
            b = (cx + direction[0] * HALF_LENGTH, cz + direction[1] * HALF_LENGTH)
            distance = segment_distance_sq(previous, pos, a, b)
            if distance <= HIT_RADIUS_SQ and (nearest is None or distance < nearest[0]):
                nearest = (distance, grid, kind)
        if nearest is None:
            last_reflector = None
        elif nearest[1] != last_reflector:
            last_reflector = nearest[1]
            vel[:] = (-vel[1], -vel[0]) if nearest[2] == "/" else (vel[1], vel[0])

        kind = gimmicks.get(stage.grid(pos))
        if kind and kind.startswith("P"):
            grid = stage.grid(pos)
            if grid != last_panel:
                arrows = {"P+Z": (0, 1), "P+X": (1, 0), "P-Z": (0, -1), "P-X": (-1, 0)}
                arrow = arrows[kind]
                magnitude = math.hypot(*vel)
                if (vel[0] * arrow[0] + vel[1] * arrow[1]) / magnitude >= math.sqrt(0.5):
                    vel[0] *= 1.5
                    vel[1] *= 1.5
                    last_panel = grid
        else:
            last_panel = None

        if stage.grid(pos) == stage.goal:
            return frame + 1
        if math.hypot(*vel) < STOP_SPEED:
            return None
    return None


def solve(stage: Stage):
    starts = range(max(0, stage.start[0] - 1), min(stage.w - 1, stage.start[0] + 1) + 1)
    # Marked cells are the authored puzzle slots. Search reflection layouts first,
    # then include directional acceleration panels if necessary.
    type_sets = [("/", "\\")]
    if len(stage.marked) <= 3:
        type_sets.append(("/", "\\", "P+Z", "P+X", "P-Z", "P-X"))
    for types in type_sets:
        for layout in itertools.product(types, repeat=len(stage.marked)):
            gimmicks = dict(zip(stage.marked, layout))
            for start_x in starts:
                angles = sorted(range(-15, 16), key=abs)
                speeds = sorted((1.25 + step * 0.125 for step in range(31)), key=lambda value: abs(value - 3.5))
                for angle in angles:
                    for speed in speeds:
                        frames = simulate(stage, start_x, angle, speed, gimmicks)
                        if frames:
                            return start_x, angle, speed, gimmicks, frames
    return None


def main():
    stages_root = Path(__file__).parents[1] / "Resources" / "Stages"
    patterns = sys.argv[1:] or ["*/*.csv"]
    paths = sorted({p for pattern in patterns for p in stages_root.glob(pattern) if "Tutorial" not in str(p)})
    failed = []
    for path in paths:
        stage = Stage(path)
        result = solve(stage)
        print(f"{path.parent.name}/{path.name}: {result}", flush=True)
        if result is None:
            failed.append(path)
    raise SystemExit(1 if failed else 0)


if __name__ == "__main__":
    main()
