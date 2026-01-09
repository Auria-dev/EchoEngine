# EchoEngine

A modern 3D rendering engine built with C++20 and OpenGL 4.6, demonstrating physically-based rendering (PBR) techniques with a deferred rendering pipeline (and forward for transparent objects)

![EchoEngine Screenshot](Image.png)

## Features

### Rendering Pipeline
- **Deferred / Forward**: Opaque objects use deferred shading; transparent objects use forward rendering
- **Physically-Based Rendering**: Cook-Torrance BRDF with metallic-roughness workflow
- **Image-Based Lighting**: Environment cubemap, irradiance map, prefilter map, BRDF LUT
- **Screen-Space Ambient Occlusion**: Kernel-based SSAO with bilateral blur
- **Skybox Rendering**: Environment cubemap with seamless transitions

### Asset Management
- **OBJ/MTL Model Loading**: Full support for Wavefront OBJ format with material libraries
- **Texture Loading**: PNG/JPG support via STBImage
- **Resource Caching**: Lazy loading and GPU upload on-demand

### Tools & Debugging
- **Debug UI**: ImGui-based scene inspector and texture debugger
- **Performance Monitor**: Real-time FPS counter and frame time graph
- **Hot Reloading**: Live shader reloading with R key
- **GPU Texture Debugger**: Visualize G-Buffer targets, SSAO outputs, and material cache

## Quick Start

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.16 or higher
- OpenGL 4.6 compatible graphics driver

### Building

#### Linux/macOS
```bash
git clone https://github.com/Auria-dev/EchoEngine.git
cd EchoEngine
chmod +x run.sh
./run.sh
```

#### Windows
```batch
git clone https://github.com/Auria-dev/EchoEngine.git
cd EchoEngine
run.bat
```

#### Manual Build
```bash
mkdir build && cd build
cmake .. -G Ninja
ninja
./EchoEngine
```

## Controls

| Action | Key |
|--------|-----|
| Move Forward | W |
| Move Backward | S |
| Move Left | A |
| Move Right | D |
| Move Up | Space |
| Move Down | Left Shift |
| Toggle Cursor | Enter |
| Fullscreen | F11 |
| Reload Shaders | R |
| Quit | Escape |

## Architecture

EchoEngine follows a modular architecture with clear separation of concerns:

```mermaid
graph TB
    subgraph "Application Layer"
        App["Application<br/>(Application.cpp)"]
        Input["InputManager<br/>Keyboard/Mouse Events"]
    end
    
    subgraph "Scene Organization"
        Scene["Scene Data Structure<br/>Scene.h"]
        Entity["Entity<br/>Renderable Objects"]
        Camera["Camera<br/>View/Projection"]
        Lights["Light System<br/>Dir/Point/Spot"]
    end
    
    subgraph "Rendering Pipeline"
        Renderer["Renderer<br/>(Renderer.cpp)"]
        Shader["Shader Programs<br/>GLSL Compilation"]
        GBuffer["G-Buffer<br/>Position/Normal/Albedo/ARM"]
        RenderTex["RenderTexture<br/>GPU Texture Wrapper"]
    end
    
    subgraph "Resource Management"
        OBJLoader["OBJLoader<br/>Wavefront Parser"]
        Mesh["Mesh/SubMesh<br/>Geometry Data"]
        Material["Material<br/>Texture Aggregation"]
        Texture["Texture<br/>CPU-side Images"]
    end
    
    App --> Renderer
    App --> Input
    App --> Scene
    Renderer --> Scene
    Renderer --> Shader
    Renderer --> GBuffer
    Scene --> Entity
    Scene --> Camera
    Scene --> Lights
    Entity --> Mesh
    Entity --> Material
    OBJLoader --> Mesh
    OBJLoader --> Material
    Material --> Texture
```

### Core Components

- **Application Class**: Main orchestrator handling window management, input, and the main loop
- **Renderer**: Multi-pass rendering pipeline with G-Buffer, SSAO, lighting, and forward passes
- **Scene System**: Entity-component style scene organization with lights and camera
- **Resource Management**: Lazy loading with CPU/GPU resource separation

## Rendering Pipeline

The engine implements a multi-pass rendering pipeline:

1. **G-Buffer Pass**: Render position, normal, albedo, and ARM (Ambient/Roughness/Metallic) to separate render targets
2. **SSAO Generation**: Compute ambient occlusion using position/normal from G-Buffer
3. **SSAO Blur**: Apply bilateral filter to smooth SSAO result
4. **Lighting Pass**: Fullscreen quad applying PBR lighting with IBL
5. **Skybox Rendering**: Draw environment cubemap
6. **Forward Pass**: Render transparent objects with alpha blending

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| GLFW | 3.x | Window creation and input handling |
| Glad | - | OpenGL 4.6 function loader |
| GLM | - | Mathematics library for 3D graphics |
| ImGui | Docking Branch | Debug UI and scene inspector |
| STBImage | - | Image loading (PNG/JPG) |

## Build System

The CMake configuration includes several optimizations for fast compilation:

- **C++20 Standard**: Modern C++ features
- **Unity Builds**: Reduce compilation time
- **Precompiled Headers**: Cache common includes
- **CCache**: Build caching across recompiles
- **Fast Linker**: Prefer mold > lld > default

## Asset Credits

All "heavy" (large) models are from [jvm-graphics-labs/awesome-3d-meshes](https://github.com/jvm-graphics-labs/awesome-3d-meshes/)

## Future Plans

- [ ] Transparency support (split opaque and translucent meshes)
- [ ] Binary compression of meshes
- [ ] Texture block compression
- [ ] Primitive drawing/generation
- [ ] 3D Billboard images
- [ ] 3D Gizmos and entity selection
- [ ] Asset manager GUI
- [ ] Atmospheric scattering
- [ ] Volumetric rendering (clouds, lightshafts)
- [ ] Post-processing effects (bloom, motion blur, depth of field)
- [ ] Multithreading (for loading assets, rendering, etc)
- [ ] Save state system / hot reloading
- [ ] Animation system

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

## License

This project is provided as-is for educational and demonstration purposes.

---

> GLAD configuration used: [glad.dav1d.de](https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.6&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=compatibility&loader=on)
