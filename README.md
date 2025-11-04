# Voxel Render in OpenGL

![alpha build](https://d3crypt3d.com/public/Screenshot%202025-10-30%20220506.png)

## Linux

### Dependencys

- libglfw3-dev

## Build

```
make <[dbg|default|clean]>
```

## TODO

- [x] Real Worker Queue
- [x] Skybox
- [ ] Debug Mode
  - [ ] Debug Tree (to chose what to enable/disable)
  - [x] F3 Wireframe Toggle
  - [ ] Chunk borders
  - [ ] Coordinates
- [ ] Optimize Chunk on VRAM
  - [x] LODs
    - [ ] better LODs
  - [x] Remove not Visible Blocks
    - [x] Frustum Culling
  - [ ] Greedy Meshing
- [ ] Interact with the world
  - [ ] Break Blocks
  - [ ] Place Blocks
    - [ ] Select Block
  - [ ] Hightlight selected Block
