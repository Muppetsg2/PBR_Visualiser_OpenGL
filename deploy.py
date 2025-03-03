import os
import shutil
import re
from colorama import Fore, Style

APP_NAME = "PBR_Visualiser"
VERSION_FILE = "src\\version.h"
DEPLOY_FOLDER = "out\\deploy"

def read_version_from_file(file_path: str) -> str:
    if not os.path.exists(file_path):
        print(f"The file '{file_path}' does not exist.")
        return "0.0.0"
    
    with open(file_path, 'r', encoding='utf-8') as file:
        content = file.read()
    
    major = re.search(r"#define PBR_VISUALISER_VER_MAJOR (\d+)", content)
    minor = re.search(r"#define PBR_VISUALISER_VER_MINOR (\d+)", content)
    patch = re.search(r"#define PBR_VISUALISER_VER_PATCH (\d+)", content)
    
    if major and minor and patch:
        return f"{major.group(1)}.{minor.group(1)}.{patch.group(1)}"
    else:
        print("Failed to read program version from file.")
        return "0.0.0"

def copy_file(source: str, target_dir: str) -> str | None:
    if not os.path.exists(source):
        print(f"The source file '{source}' does not exist.")
        return None
    
    os.makedirs(target_dir, exist_ok=True)

    if os.path.isfile(source):
        name = os.path.basename(source)
        target_path = os.path.join(target_dir, name)
        
        shutil.copy2(source, target_path)
        print(f"{Fore.YELLOW}Copied{Style.RESET_ALL} '{name}' to '{target_path}'")
        return target_path
    
    return None

def copy_folder(source_folder: str, destination_folder: str) -> str | None:
    if not os.path.exists(source_folder):
        print(f"The source folder '{source_folder}' does not exist.")
        return None
    
    try:
        shutil.copytree(source_folder, destination_folder)
        print(f"{Fore.YELLOW}Copied{Style.RESET_ALL} '{source_folder}' folder to '{destination_folder}'")
        return destination_folder
    except FileExistsError:
        print(f"The destination folder '{destination_folder}' already exists. Choose a different name or delete the existing folder.")
        return None
    except Exception as e:
        print(f"An error occurred while copying: {e}")
        return None
    
def delete_path(path: str):
    if not os.path.exists(path):
        print(f"The path '{path}' does not exist.")
        return
    
    if os.path.isfile(path):
        os.remove(path)
        print(f"{Fore.RED}Deleted{Style.RESET_ALL} '{path}' file")
    elif os.path.isdir(path):
        shutil.rmtree(path)
        print(f"{Fore.RED}Deleted{Style.RESET_ALL} '{path}' folder")

def move_file(source_file: str, destination_folder: str):
    if not os.path.exists(source_file):
        print(f"The source file '{source_file}' does not exist.")
        return
    
    if not os.path.exists(destination_folder):
        os.makedirs(destination_folder)
    
    destination_file = os.path.join(destination_folder, os.path.basename(source_file))
    shutil.move(source_file, destination_file)
    
    parent_folder = os.path.dirname(source_file)
    while parent_folder and os.path.exists(parent_folder) and not os.listdir(parent_folder):
        os.rmdir(parent_folder)
        parent_folder = os.path.dirname(parent_folder)
    
    print(f"{Fore.CYAN}Moved{Style.RESET_ALL} '{source_file}' to '{destination_folder}'")

def rename_path(old_path: str, new_path: str):
    if not os.path.exists(old_path):
        print(f"The path '{old_path}' does not exist.")
    
    os.rename(old_path, new_path)
    print(f"{Fore.MAGENTA}Changed{Style.RESET_ALL} '{old_path}' to '{new_path}'")

if __name__ == "__main__":
    deploy_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), DEPLOY_FOLDER)
    version = read_version_from_file(os.path.join(os.path.dirname(os.path.abspath(__file__)), VERSION_FILE))

    builds = {
        "Window_x64": {
            "source": os.path.join(os.path.dirname(os.path.abspath(__file__)), 'out\\build\\x64-Window\\src'),
            "files": [
                "PBR_Visualiser.exe",
                "shaders.dat",
                "imgui.ini",
                "icon.png",
                "res\\skybox\\rooitou_park_4k.hdr",
                "res\\model\\glft\\lamp.glb"
            ],
            "folders": [
                "res\\skybox\\Park",
                "res\\textures\\Rock051_2K-PNG",
                "res\\model\\textures"
            ],
            "toRemove": [
                "res\\textures\\Rock051_2K-PNG\\Rock051.png",
                "res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_NormalDX.png"
            ],
            "toMove": [
                ("res\\model\\glft\\lamp.glb", "res\\model\\lamp.glb")
            ],
            "changeName": [
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_AmbientOcclusion.png", "res\\textures\\Rock051_2K-PNG\\AmbientOcclusion.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Color.png", "res\\textures\\Rock051_2K-PNG\\Color.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Displacement.png", "res\\textures\\Rock051_2K-PNG\\Displacement.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Metalness.png", "res\\textures\\Rock051_2K-PNG\\Metalness.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_NormalGL.png", "res\\textures\\Rock051_2K-PNG\\Normal.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Roughness.png", "res\\textures\\Rock051_2K-PNG\\Roughness.png")
            ]
        },
        "Console_x64": {
            "source": os.path.join(os.path.dirname(os.path.abspath(__file__)), 'out\\build\\x64-Console\\src'),
            "files": [
                "PBR_Visualiser.exe",
                "shaders.dat",
                "icon.png",
                "res\\skybox\\brown_photostudio_01_4k.hdr",
                "res\\skybox\\golden_bay_4k.hdr",
                "res\\skybox\\hilly_terrain_01_4k.hdr",
                "res\\skybox\\modern_bathroom_4k.hdr",
                "res\\skybox\\moonless_golf_4k.hdr",
                "res\\skybox\\rooitou_park_4k.hdr",
                "res\\skybox\\satara_night_4k.hdr",
                "res\\skybox\\snowy_field_4k.hdr",
                "res\\skybox\\venice_sunset_4k.hdr"
            ],
            "folders": [
                "res\\textures\\Rock051_2K-PNG"
            ],
            "toRemove": [
                "res\\textures\\Rock051_2K-PNG\\Rock051.png",
                "res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_NormalDX.png"
            ],
            "toMove": [],
            "changeName": [
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_AmbientOcclusion.png", "res\\textures\\Rock051_2K-PNG\\AmbientOcclusion.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Color.png", "res\\textures\\Rock051_2K-PNG\\Color.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Displacement.png", "res\\textures\\Rock051_2K-PNG\\Displacement.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Metalness.png", "res\\textures\\Rock051_2K-PNG\\Metalness.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_NormalGL.png", "res\\textures\\Rock051_2K-PNG\\Normal.png"),
                ("res\\textures\\Rock051_2K-PNG\\Rock051_2K-PNG_Roughness.png", "res\\textures\\Rock051_2K-PNG\\Roughness.png")
            ]
        }
    }

    os.makedirs(deploy_path, exist_ok=True)

    for name, data in builds.items():
        print(f"Creating deploy folder for '{name}' build")
        save_path = os.path.join(deploy_path, f"{APP_NAME}_{version}_{name}")
        os.makedirs(save_path, exist_ok=True)

        for f in data["files"]:
            copy_file(os.path.join(data["source"], f), os.path.join(save_path, os.path.dirname(f)))

        for fl in data["folders"]:
            copy_folder(os.path.join(data["source"], fl), os.path.join(save_path, fl))

        for r in data["toRemove"]:
            delete_path(os.path.join(save_path, r))
        
        for mFrom, mTo in data["toMove"]:
            move_file(os.path.join(save_path, mFrom), os.path.join(save_path, os.path.dirname(mTo)))

        for old, new in data["changeName"]:
            rename_path(os.path.join(save_path, old), os.path.join(save_path, new))
        
        print(f"{Fore.GREEN}Finished{Style.RESET_ALL} creating deploy folder for '{name}' build")