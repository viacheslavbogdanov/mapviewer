uniform vec3 MeshColor;

varying vec3 DiffuseLight;

void main()
{
    gl_FragColor.rgb = MeshColor.rgb * DiffuseLight;
    gl_FragColor.a = 1.0;
}
