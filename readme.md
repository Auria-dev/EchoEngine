# EchoEngine
This is a work in progress

<img src="Image.png">

> All "heavy" (large) models are from https://github.com/jvm-graphics-labs/awesome-3d-meshes/

<br /> 

# Current features
- Input manager
- Custom OBJ loader
- Deferred rendering
- SSAO
- PBR & IBL lighting
- Skybox
- 3 Light primitives

# Plans for the future
- Transparency support (split opaque and translucent meshes)
- Binary compression of meshes
- Texture block compression
- Primitive drawing/generation
- 3D Billboard images
- 3D Gizmos and entity selection
- Asset manager gui
- [Atmospheric scattering](https://sebh.github.io/publications/egsr2020.pdf)
- Volumetric rendering (clouds, lightshafts)
- Post-processing effects (bloom, motion blur, depth of field)
- Multithreading (for loading assets, rendering, etc)
- Save state system / hot reloading
- Animation system


<br />

> You can find the GLAD configuration I used [here](https://glad.dav1d.de/#language=c&specification=gl&api=gl%3D4.6&api=gles1%3Dnone&api=gles2%3Dnone&api=glsc2%3Dnone&profile=compatibility&loader=on)