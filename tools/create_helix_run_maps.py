"""Generate layout binaries for the Helix Nexus / Caves run maps.

Two 11x9 rooms using gTileset_General (primary) + gTileset_Cave (secondary):
  - HelixNexus:    hub room (guide, PC, three doors)
  - HelixCaveRoom: shared by the intro/wild/trainer/shop/boss cave rooms

Tile words: metatile | (collision << 10) | (elevation << 12)
  FLOOR = metatile 0x201 (cave floor, MB_CAVE), passable, elevation 3
  WALL  = metatile 0x219 (cave wall), impassable
Door gaps are floor tiles in the wall ring at top (5,0), left (0,4) and
right (10,4); coord events on those tiles drive door logic in scripts.
"""
import struct
import os

base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

WIDTH = 11
HEIGHT = 9
FLOOR = 0x0201 | (0 << 10) | (3 << 12)  # 0x3201
WALL = 0x0219 | (1 << 10) | (0 << 12)   # 0x0619
GAPS = [(5, 0), (0, 4), (10, 4)]        # top, left, right door tiles


def build_room():
    tiles = []
    for y in range(HEIGHT):
        for x in range(WIDTH):
            is_edge = x == 0 or y == 0 or x == WIDTH - 1 or y == HEIGHT - 1
            if is_edge and (x, y) not in GAPS:
                tiles.append(WALL)
            else:
                tiles.append(FLOOR)
    return tiles


def write_layout(name):
    layout_dir = os.path.join(base, "data", "layouts", name)
    os.makedirs(layout_dir, exist_ok=True)
    with open(os.path.join(layout_dir, "border.bin"), "wb") as f:
        f.write(struct.pack("<4H", WALL, WALL, WALL, WALL))
    tiles = build_room()
    with open(os.path.join(layout_dir, "map.bin"), "wb") as f:
        f.write(struct.pack("<%dH" % len(tiles), *tiles))
    print("wrote", layout_dir)


write_layout("HelixNexus")
write_layout("HelixCaveRoom")
print("Helix run map binary data created successfully.")
