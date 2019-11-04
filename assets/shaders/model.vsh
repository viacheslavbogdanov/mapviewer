attribute vec4 inVertex;
attribute vec4 inNormal;

uniform mat4 MVPMatrix;

varying vec3 DiffuseLight;

void main()
{
    gl_Position = MVPMatrix * inVertex;
    DiffuseLight = vec3(max(dot(inNormal, vec3(0.0, 0.0, 1.0)), 0.0));
}
