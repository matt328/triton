#pragma once

namespace tr {
// This is just a tag interface for Handle<> that is internally converted into a
// Handle<GeometryRegion> used by the Renderer
struct Geometry {};

/// Tag interface for externally referencing a Handle<Texture>
struct TextureTag {};
}
