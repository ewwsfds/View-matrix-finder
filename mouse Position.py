import pyautogui
import keyboard

print("Press K to print mouse coordinates...")

while True:
  
    keyboard.wait('k')
    x, y = pyautogui.position()
    print(f"X={x} Y={y}")


