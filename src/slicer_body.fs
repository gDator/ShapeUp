in vec2 fragTexCoord;
out vec4 finalColor;
uniform float z;

void main()
{
    vec4 sdf_value = signed_distance_field(vec3(fragTexCoord, z));
    finalColor =  vec4(sdf_value.x, sdf_value.y, sdf_value.z, sdf_value.w);//vec4(sdf_value.y, sdf_value.z, sdf_value.w, sdf_value.x);
}
