import os
import sys
import tkinter as tk
from threading import Thread
import shutil
import time

unity_path : str = ""
UNITY_PATHES = 'UnityPathes.txt'
EXPORT_PATH = '//Export//'

def get_items_in_folder(folder_path):
    items = []
    # 폴더 내의 모든 파일과 폴더의 리스트를 얻음
    contents = os.listdir(folder_path)
    for item in contents:
        items.append(item)
    return items


def update_ui():
    root.after(1000, update_ui)

def update_text(message):
    text.insert(tk.END, message + '\n')

def copy_folder():
    global root
    crnt_dir = os.getcwd() + '\\'
    items = get_items_in_folder(unity_path)
    try:
        for item in items:
            item_path = unity_path + item
            update_text(item)
            if os.path.isfile(item_path):
                shutil.copy(item_path, crnt_dir)
            else:
                shutil.copytree(item_path, crnt_dir + item, dirs_exist_ok=True)
    except Exception as e:
        update_text('===[import error]===\n' + str(e))
        exit()

def import_scene():
    copy_folder()
    update_text('\nimport completed.')
    update_text('3초 후 자동으로 종료됩니다.')
    time.sleep(1)
    update_text('2초 후 자동으로 종료됩니다.')
    time.sleep(1)
    update_text('1초 후 자동으로 종료됩니다.')
    time.sleep(1)
    root.after(10, root.destroy)

def start_import():
    thread = Thread(target=import_scene)
    thread.start()
    


with open(UNITY_PATHES, 'r') as file:
    for line in file:
        unity_path = line.strip()
        unity_path += EXPORT_PATH
        if(os.path.exists(unity_path)):
            break
        unity_path = ""

root = tk.Tk()
root.title("Scene Importer")
windows_width = root.winfo_screenwidth()
windows_height = root.winfo_screenheight()
app_width = 700
app_height = 300
center_x = int((windows_width / 2) - (app_width / 2))
center_y = int((windows_height / 2) - (app_height / 2))
root.geometry(f"{app_width}x{app_height}+{center_x}+{center_y}")

text = tk.Text(root)

file_name = os.path.basename(sys.argv[0])
file_name = file_name[:-3]
text.insert(tk.INSERT, file_name + '\n')

if(unity_path == ""):
    text.insert(tk.INSERT, '주어진 경로들(' + UNITY_PATHES + ') 중에서 폴더를 찾을 수 없습니다.\n')
else:
    text.insert(tk.INSERT, 'Unity Path : ' + unity_path + '\n')
    text.insert(tk.INSERT, 'Import...\n\n')

text.pack(fill='x')

update_ui()
start_import()

root.mainloop()