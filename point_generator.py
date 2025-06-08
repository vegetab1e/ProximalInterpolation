import random
import json

from itertools import chain

# Создание множества опорных точек
known_points = set()
while len(known_points) < 300:
    x = random.randint(-100, 100)
    y = random.randint(-100, 100)
    if (x, y) not in known_points:
        value = round(random.uniform(-100, 100), 8)
        known_points.add((x, y, value))

# Преобразование в формат JSON
known_points = [{"x": x, "y": y, "value": value} for (x, y, value) in known_points]

# Запись в файл
with open('known_points.json', 'w') as file:
    json.dump(known_points, file, indent = 4)

# Создание множества искомых точек
unknown_points = set()
while len(unknown_points) < 10:
    x = random.randint(-100, 100)
    y = random.randint(-100, 100)
    if (x, y) not in chain(known_points, unknown_points):
        unknown_points.add((x, y))

# Преобразование в формат JSON
unknown_points = [{"x": x, "y": y} for (x, y) in unknown_points]

# Запись в файл
with open('unknown_points.json', 'w') as file:
    json.dump(unknown_points, file, indent = 4)
