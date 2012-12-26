#version 330 core

uniform sampler2D texSampler;

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

in Vertex
{
    flat vec3 normal;
    flat vec4 specular;
    vec3 position;

    vec4 light;
    vec2 texCoord;
    float under;
}
vertex;

out vec4 color;

// Schlick's approximation a ^ b == a / (b – a * b + a)
// a in [0.0; 1.0].
float approxPow(float a, float b)
{
    return a / (b - a * b + a);
}

vec4 getSpecularLight()
{
    vec3 to_light = pointLight.position - vertex.position;
    vec3 to_light_norm = normalize(to_light);
    vec3 to_camera = transform.viewPosition - vertex.position;
    vec3 to_camera_norm = normalize(to_camera);

    float RdotV = max(dot(reflect(-to_light_norm, vertex.normal),
        to_camera_norm), 0.0);

    RdotV = approxPow(RdotV, material.shininess);

    return vertex.specular * RdotV;
}

float water_color_factor(float water_thickness)
{
    const float e = 2.718281828459045235360;
    return clamp((exp(water_thickness * e / 25.0) - 1.0), 0.0, 0.5);
}

void main(void)
{
    vec4 waterColor = vec4(0.0, 0.20, 0.40, 1.0);
    vec4 specular_light = getSpecularLight();
    vec4 light = (vertex.light + specular_light);
    vec4 objColor = texture(texSampler, vertex.texCoord);

    if (vertex.under > 0.0)
    {
        color = objColor * light;
    }
    else
    {
        float dist = distance(vertex.position, transform.viewPosition);
        float wf = water_color_factor(dist);
        color = light * mix(objColor, waterColor, wf);
    }
}
