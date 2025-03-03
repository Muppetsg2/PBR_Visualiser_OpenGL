# 🖥️ PBR Visualiser Console - Examples

## 1️⃣ Generating a PBR image with 5 maps in the correct order
```bash
PBR_Visualiser.exe -f
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Color.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_NormalGL.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Metalness.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Displacement.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Roughness.png
```

## 2️⃣ Generating a PBR image with 6 maps (missing Metalness)
```bash
PBR_Visualiser.exe -f
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_Color.png
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_NormalGL.png
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_Metalness.png
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_Displacement.png
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_Roughness.png
    .\res\textures\RoofingTiles013A_2K-PNG\RoofingTiles013A_2K-PNG_AmbientOcclusion.png
```

## 3️⃣ Generating a PBR image with additional console information
```bash
PBR_Visualiser.exe -v -f
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Color.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_NormalGL.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Metalness.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Displacement.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Roughness.png
```

## 4️⃣ Generating a PBR image with the plane position set to `right`
```bash
PBR_Visualiser.exe -v -p right -f
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Color.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_NormalGL.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Metalness.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Displacement.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Roughness.png
```

## 5️⃣ Generating a PBR image with additional parameters
**Options:**
- 🖼️ File Name: `Name.png`
- 📂 Save Folder: `res`
- 🌅 Skybox: `photostudio`
- 🖥️ Resolution: `1024x1024`
- ☀️ Exposure: `2.0`
- 🎨 Color Intensity: `1.0`
```bash
PBR_Visualiser.exe -v -p right -f
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Color.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_NormalGL.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Metalness.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Displacement.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Roughness.png
    -n Name -d .\res -s photostudio -r r1k -e 2.0 -i 1.0
```

## 6️⃣ Start program in interactive mode
```bash
PBR_Visualiser.exe -I
```

## 7️⃣ Generating a PBR image in interactive mode
```bash
PBR_Visualiser.exe -v -I -p right -f
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Color.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_NormalGL.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Metalness.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Displacement.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Roughness.png
```

## 8️⃣ Generating a PBR image in interactive mode with 4 maps and the name `Hello.png`
```bash
PBR_Visualiser.exe -v -I -p right -f
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Color.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_NormalGL.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Metalness.png
    .\res\textures\Rock051_2K-PNG\Rock051_2K-PNG_Displacement.png
    -n Hello
```

## 9️⃣ Generating a PBR image of a metallic material in interactive mode
```bash
PBR_Visualiser.exe -v -p right -I -f
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Color.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_NormalGL.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Metalness.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Displacement.png
    .\res\textures\Metal049A_2K-PNG\Metal049A_2K-PNG_Roughness.png
    -n Name -d .\res -s photostudio -r r1k -e 2.0 -i 1.0
```