import struct
import os

base = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# HelixIntroRoom: 7x7 indoor room
intro_dir = os.path.join(base, "data", "layouts", "HelixIntroRoom")
os.makedirs(intro_dir, exist_ok=True)
with open(os.path.join(intro_dir, "border.bin"), "wb") as f:
    f.write(struct.pack("<4H", 0x0202, 0x0202, 0x0202, 0x0202))
with open(os.path.join(intro_dir, "map.bin"), "wb") as f:
    f.write(struct.pack("<49H", *([0x0201] * 49)))

# HelixIsland: 15x15 outdoor area
island_dir = os.path.join(base, "data", "layouts", "HelixIsland")
os.makedirs(island_dir, exist_ok=True)
with open(os.path.join(island_dir, "border.bin"), "wb") as f:
    f.write(struct.pack("<4H", 0x0001, 0x0001, 0x0001, 0x0001))
with open(os.path.join(island_dir, "map.bin"), "wb") as f:
    f.write(struct.pack("<225H", *([0x0001] * 225)))

print("Map binary data created successfully.")
