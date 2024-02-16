import shutil

SCENE_PATH = "E:/University/Unity/SceneLoader/"

scene_file = "Scene.bin"
scene_src = SCENE_PATH + scene_file

shutil.copyfile(scene_src, scene_file)

mesh_folder = "Meshes"
mesh_src = SCENE_PATH + mesh_folder
shutil.copytree(mesh_src, mesh_folder, dirs_exist_ok=True)