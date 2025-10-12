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
- [ ] Optimize Chunk on VRAM
  - [x] Remove not Visible Blocks
  - [ ] Only send Faces visible from player Chunk to VRAM
- [ ] Interact with the world
  - [ ] Break Blocks
  - [ ] Place Blocks
    - [ ] Select Block
  - [ ] Hightlight selected Block
