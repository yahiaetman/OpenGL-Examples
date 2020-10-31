#version 330 core
// The first line of any shader must a '#version" command which defines the glsl version to use

// This is entry point of the vertex shader and it will be called for every vertex of our geometry
void main() {
    // For this shader, we won't send any geometry data from the C++ code
    // so we will create the geometry data here
    // Therefore, we create an array of 3 positions (3x 3D vectors) and we will pick one later based on the vertex ID
    const vec3 positions[3] = vec3[3](
        vec3( 0.5, -0.5, 0.0),
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );

    // Pick a vertex position based on the builtin variable "gl_VertexID" which contains the index of the vertex
    // Then we convert it into a 4D vector where the 4th component 'w' is equal to 1 (w is called the homogenous component).
    // Finally we pass the value to a builtin variable called "gl_Position" which is used by OpenGL to determine the vertex position on the screen
    gl_Position = vec4(positions[gl_VertexID], 1.0);

    // If you're wondering why we have a homogenous component,
    // it is used later to do perspective projection (which make objects look smaller the farther they are).
    // Simply put, OpenGL will divide (x,y,z) by (w) so if we make (w) directly proportional to the distance of the vertex, we'll get perspective projection.

    // If you're wonder why the position coordinates are small (0.5 on x & y) and why some of them are negative,
    // Then you need to know about "Normalized Device Coordinates" (NDC).
    // These are the coordinates in which "gl_Position" is specified.
    // On the screen (or the viewport to be more accurate), the NDC extend from -1 to 1 on each coordinate.
    // So the NDC will look like this on the screen:
    //
    //      (-1, 1)     --      ( 0, 1)     --      ( 1, 1)
    //          |                   |                   |
    //          |                   |                   |
    //      (-1, 0)     --      ( 0, 0)     --      ( 1, 0)
    //          |                   |                   |
    //          |                   |                   |
    //      (-1,-1)     --      ( 0,-1)     --      ( 1,-1)
    //
    // Note that (0, 0) is the center and the Y-Axis points upwards.
    // It is note-worthy that anything outside the range -1 to 1 on the NDC is clipped (not rendered)
    // So a triangle can look like a different polygon if any of its vertics extend about the range due to clipping
    // This also applies to the Z-Axis so it is better to visualize the NDC space as a cube rather than a square
    // even though the Z-Axis will disappear when the result is drawn on the screen (since it is flat).
    // so the NDC space boundaries can be seen as:
    //              (-1, 1, 1)-------------( 1, 1, 1)
    //             /|                     /|
    //            / |                    / |
    //          (-1, 1,-1)-------------( 1, 1,-1)
    //          |   |                   |  |
    //          |  (-1,-1, 1)-----------|--( 1,-1, 1)
    //          | /                     | /
    //          |/                      |/
    //          (-1,-1,-1)-------------( 1,-1,-1)
}