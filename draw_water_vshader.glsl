#version 330 core

in vec3 position;

uniform vec2 meshTexStep;
uniform vec2 meshViewFirst;
uniform vec2 meshViewSize;

uniform sampler2D texGeometry;

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

out Vertex
{
    flat vec4 specular;
    vec3 position;
    vec4 light;
    float under;
}
vertex;

vec4 calcLight(vec3 real_normal, vec3 to_camera_norm)
{
    vec4 ambient = material.ambient * pointLight.ambient;
    vec4 diffuse = material.diffuse * pointLight.diffuse;

    vec3 to_light = pointLight.position - position;
    vec3 to_light_norm = normalize(to_light);

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

vec4 calcPosition(vec2 xy, out vec2 texCoord)
{
    vec2 tc = (xy - meshViewFirst) / meshViewSize;
    float z = texture(texGeometry, tc).r;

    texCoord = tc;
    return vec4(xy, z, 1.0);
}

#if 0
// tc -- texture coordinates.
vec3 calcNormal(vec2 tc, vec3 to_camera_norm)
{
    float zL = texture(texGeometry, vec2(tc.x - meshTexStep.x, tc.y)).r;
    float zR = texture(texGeometry, vec2(tc.x + meshTexStep.x, tc.y)).r;
    float zD = texture(texGeometry, vec2(tc.x, tc.y - meshTexStep.y)).r;
    float zU = texture(texGeometry, vec2(tc.x, tc.y + meshTexStep.y)).r;

    vec3 dx = vec3(2.0 * meshTexStep.x, 0.0, zR - zL);
    vec3 dy = vec3(0.0, 2.0 * meshTexStep.y, zU - zD);

    //vec3 normal = normalize(vec3(zL - zR, zD - zU, 2.0 * texStep.x));
    vec3 normal = normalize(cross(dx, dy));

    if (dot(to_camera_norm, normal) < 0.0)
    {
        normal = -normal;
    }

    return normal;
}
#endif

// tc -- texture coordinates.
vec3 calcNormal(vec2 tc, vec3 to_camera_norm)
{
    float zL = texture(texGeometry, vec2(tc.x - meshTexStep.x, tc.y)).r;
    float zR = texture(texGeometry, vec2(tc.x + meshTexStep.x, tc.y)).r;
    float zD = texture(texGeometry, vec2(tc.x, tc.y - meshTexStep.y)).r;
    float zU = texture(texGeometry, vec2(tc.x, tc.y + meshTexStep.y)).r;

    float zLL = texture(texGeometry, vec2(tc.x - meshTexStep.x*3, tc.y)).r;
    float zRR = texture(texGeometry, vec2(tc.x + meshTexStep.x*3, tc.y)).r;
    float zDD = texture(texGeometry, vec2(tc.x, tc.y - meshTexStep.y*3)).r;
    float zUU = texture(texGeometry, vec2(tc.x, tc.y + meshTexStep.y*3)).r;

    vec3 dx = vec3(pow(2.0 * meshTexStep.x, 2) + 6.0 * meshTexStep.x, 0.0, pow(zR - zL, 2) + (zRR - zLL));
    vec3 dy = vec3(0.0, pow(2.0 * meshTexStep.y, 2) + 6.0 * meshTexStep.y, pow(zU - zD, 2) + (zUU - zDD));

    //vec3 normal = normalize(vec3(zL - zR, zD - zU, 2.0 * texStep.x));
    vec3 normal = normalize(cross(dx, dy));

    if (dot(to_camera_norm, normal) < 0.0)
    {
        normal = -normal;
    }

    return normal;
}

void main(void)
{
    vec2 texCoord;

    vec3 to_camera = transform.viewPosition - position;
    vec3 to_camera_norm = normalize(to_camera);

    vec4 real_position = calcPosition(position.xy, texCoord);

    vec3 normal = calcNormal(texCoord, to_camera_norm);

    vertex.specular = material.specular * pointLight.specular;
    vertex.position = real_position.xyz;
    vertex.light = calcLight(normal, to_camera_norm);
    vertex.under = transform.viewPosition.z -
        calcZ(transform.viewPosition.xy);

    gl_Position = transform.viewProjection * real_position;
}
