#version 330 core

in vec3 position;
in vec2 texCoord;
in vec3 normal;

uniform struct Transform
{
    mat4 viewProjection;
    vec3 viewPosition;
}
transform;

uniform struct PointLight
{
    vec3 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 attenuation;
}
pointLight;

uniform struct Material
{
    vec4 emission;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
}
material;

uniform vec2 meshViewFirst;
uniform vec2 meshViewSize;

uniform sampler2D texGeometry;

out Vertex
{
    flat vec3 normal;
    flat vec4 specular;
    vec3 position;

    vec4 light;
    vec2 texCoord;

    float under;
}
vertex;

vec4 calcLight(vec3 real_normal, vec3 to_camera)
{
    vec4 ambient = material.ambient * pointLight.ambient;
    vec4 diffuse = material.diffuse * pointLight.diffuse;

    vec3 to_light = pointLight.position - position;
    vec3 to_light_norm = normalize(to_light);
    vec3 to_camera_norm = normalize(to_camera);

    float to_light_length = length(to_light);

    float NdotL = max(dot(real_normal, to_light_norm), 0.0);

    float attenuation = clamp(1.0 / (
        pointLight.attenuation[0] +
        pointLight.attenuation[1] * to_light_length +
        pointLight.attenuation[2] * to_light_length *
            to_light_length), 0.0, 1.0);

    return attenuation * (
        material.emission +
        ambient +
        diffuse * NdotL);
}

float calcZ(vec2 xy)
{
    vec2 tc = (xy - meshViewFirst) / meshViewSize;
    float z = texture(texGeometry, tc).r;

    return z;
}

void main(void)
{
    vec3 to_camera = transform.viewPosition - position;
    vec3 real_normal = normal;

    if (dot(to_camera, real_normal) < 0)
    {
        real_normal = -real_normal;
    }

    vertex.normal = real_normal;
    vertex.specular = material.specular * pointLight.specular;
    vertex.position = position;
    vertex.light = calcLight(real_normal, to_camera);
    vertex.texCoord = texCoord;

    vertex.under = transform.viewPosition.z -
        calcZ(transform.viewPosition.xy);

    gl_Position = transform.viewProjection * vec4(position, 1.0);
}
