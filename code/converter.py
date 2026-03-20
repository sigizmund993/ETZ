import re

# Настройки: укажите имя вашего файла из Inkscape
input_file = "output.ngc" 
output_file = "code/shi.ino"

# Высота, при которой маркер поднят (из вашего G-кода это 5.0)
Z_UP_THRESHOLD = 4.5 

def convert_gcode_to_arduino(gcode_path, out_path):
    with open(gcode_path, 'r') as f:
        lines = f.readlines()

    commands = []
    commands.append("void drawShi(){")
    last_x, last_y = 0.0, 0.0
    is_pen_down = False

    for line in lines:
        line = line.strip()
        if not line or line.startswith('('): continue # Пропускаем комментарии

        # 1. Проверка подъема/опускания маркера (Z)
        z_match = re.search(r'Z([-+]?\d*\.\d+|\d+)', line)
        if z_match:
            z_val = float(z_match.group(1))
            if z_val >= Z_UP_THRESHOLD and is_pen_down:
                commands.append("raise();")
                is_pen_down = False
            elif z_val < Z_UP_THRESHOLD and not is_pen_down:
                commands.append("lower();")
                is_pen_down = True

        # 2. Поиск координат X и Y
        x_match = re.search(r'X([-+]?\d*\.\d+|\d+)', line)
        y_match = re.search(r'Y([-+]?\d*\.\d+|\d+)', line)

        if x_match or y_match:
            new_x = float(x_match.group(1)) if x_match else last_x
            new_y = float(y_match.group(1)) if y_match else last_y
            
            # Инвертируем Y, так как в Inkscape он обычно отрицательный
            commands.append(f"goToPoint({new_x:.2f}, {-new_y:.2f});")
            
            last_x, last_y = new_x, new_y
    commands.append("}")
    # Запись в файл
    with open(out_path, 'w') as f:
        f.write("\n".join(commands))
    
    print(f"Готово! Создано команд: {len(commands)}. Файл: {out_path}")

convert_gcode_to_arduino(input_file, output_file)
