# Design Notes

Need a 'frontend' to the graphics library that will abstract parts away so that the client doesn't
need to have access to the entire chain of headers the renderer class needs in order to compile. Maybe this frontend needs to be a pimpl

This is most of the way there already with the renderer returning opaque handles for created resources
and then taking those back in POD structs when being asked to render something.
