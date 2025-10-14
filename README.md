# Voxel Render in OpenGL

![alpha build](https://www.d3crypt3d.com/public/Screenshot%202025-10-10%20014410.png)

## Linux

### Dependencys

- libglfw3-dev

## Build

```
make <[dbg|default|clean]>
```

## TODO

- [ ] Real Worker Queue
- [ ] Put all Blocks into one VAO, VBO with implicit shadow Buffer
- [ ] Debug Mode
  - [ ] Debug Tree (to chose what to enable/disable)
  - [ ] F3 Mode
  - [ ] Chunk borders
  - [ ] Coordinates
  - [x] Wireframe
- [ ] Optimize Chunk on VRAM
  - [x] Remove not Visible Blocks
  - [ ] Only send Faces visible from player Chunk to VRAM
- [ ] Interact with the world
  - [ ] Break Blocks
  - [ ] Place Blocks
    - [ ] Select Block
  - [ ] Hightlight selected Block
