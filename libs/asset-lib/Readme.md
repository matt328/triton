# Asset Library

This library defines and provides an API for a custom Triton model format which uses the cereal library for serialization. It's still orders of magnitude faster than parsing gltf files, and is good enough for now.

The serialization is very naive brute force and boils down to just this:

```cpp
  template <class Archive>
  void serialize(Archive& archive, std::uint32_t const version) {
    archive(vertices, indices, jointRemaps, inverseBindPoses, imageData);
  }
```

With everything being `std::optional` except vertices and indices.

This library also defines the `as::Vertex` format that currently everything in the entire engine uses for simplicity until we have a need to optimize.

I've started to refactor the exceedingly modular code to be able to use boost:di instead of what it's currently doing, with the goal of easily supporting multiple input file formats, but for now, Blender's gltf exporting is working fine.

I'd also like to incorporate this directly in the editor so I can use file dialogs instead of struggling with long paths on the commandline.
