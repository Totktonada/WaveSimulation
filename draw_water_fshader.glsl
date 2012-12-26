#version 330 core

uniform sampler2D texGeometry;
uniform sampler2D texPool;
uniform sampler2D texCube;

uniform vec2 meshTexStep;
uniform vec2 meshViewFirst;
uniform vec2 meshViewSize;

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
    flat vec4 specular;
    vec3 position;
    vec4 light;
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

vec4 getSpecularLight(vec3 to_camera_norm, vec3 normal)
{
    vec3 to_light = pointLight.position - vertex.position;
    vec3 to_light_norm = normalize(to_light);

    vec3 h = normalize(to_light_norm + to_camera_norm);

    float HdotN = max(dot(h, normal), 0.0);

    HdotN = approxPow(HdotN, material.shininess);

    return vertex.specular * HdotN;
}

// Is point p in rectangle with left-bottom corner x and
// right-up corner y?
bool inRect(vec2 p, vec2 x, vec2 y)
{
//    return (all(greaterThanEqual(p, x)) && all(lessThanEqual(p, y)));
    return clamp(p, x, y) == p;
//    return max(p, x) == min(p, y);
}

// "refract" replacement for working with some buggy drivers/videocards.
vec3 refract_fork(vec3 I, vec3 N, float eta)
{
    float k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));

    if (k < 0.0)
    {
        return vec3(0.0, 0.0, 0.0);
    }

    return eta * I - (eta * dot(N, I) + sqrt(k)) * N;
}

float water_color_factor(float water_thickness)
{
    const float e = 2.718281828459045235360;
    return clamp((exp(water_thickness * e / 12.5) - 1.0), 0.0, 0.5);
}

// Point of intersect vector "ray" emitted from position "pos" and
// plane with x, y, or z == value (at idx == 0, 1 or 2).
void intersectK(vec3 pos, vec3 ray, int idx, float value,
    out float ak, out vec2 ap)
{
    float k = (value - pos[idx]) / ray[idx];

    int i1, i2;

    if (idx == 1)
    {
        i1 = 0;
        i2 = 2;
    }
    else
    {
        i1 = (idx + 1) % 3;
        i2 = (idx + 2) % 3;
    }

    float a = pos[i1] + ray[i1] * k;
    float b = pos[i2] + ray[i2] * k;
    ap = vec2(a, b);
    ak = k;
}

// Point of intersect vector "ray" emitted from position "pos" and
// plane with x, y, or z == value (at idx == 0, 1 or 2).
vec3 intersect(vec3 pos, vec3 ray, int idx, float value)
{
    float k = (value - pos[idx]) / ray[idx];
    return pos + ray * k;
}

struct Rect
{
    int axesOrthIdx;
    float axesValue;
    vec2 bound_p1;
    vec2 bound_p2;
    vec2 texScale;
    bool cube;
};

Rect rect[10] = Rect[10]
(
    Rect(0, -1.05, vec2(-1.05, -1.05), vec2(1.05, 1.05), vec2(1.0, 1.0), true),
    Rect(0,  1.05, vec2(-1.05, -1.05), vec2(1.05, 1.05), vec2(1.0, 1.0), true),
    Rect(1, -1.05, vec2(-1.05, -1.05), vec2(1.05, 1.05), vec2(1.0, 1.0), true),
    Rect(1,  1.05, vec2(-1.05, -1.05), vec2(1.05, 1.05), vec2(1.0, 1.0), true),
    Rect(2,  1.05, vec2(-1.05, -1.05), vec2(1.05, 1.05), vec2(1.0, 1.0), true),

    Rect(0,  10.0, vec2(-10.0, -1.0), vec2(10.0, 1.0), vec2(10.0, 1.0), false),
    Rect(0, -10.0, vec2(-10.0, -1.0), vec2(10.0, 1.0), vec2(10.0, 1.0), false),
    Rect(1,  10.0, vec2(-10.0, -1.0), vec2(10.0, 1.0), vec2(10.0, 1.0), false),
    Rect(1, -10.0, vec2(-10.0, -1.0), vec2(10.0, 1.0), vec2(10.0, 1.0), false),
    Rect(2, -1.0, vec2(-10.0, -10.0), vec2(10.0, 10.0), vec2(10.0, 10.0), false)
);

void calcRayColor(vec3 rRay, vec3 normal, out float dist, out vec4 color)
{
    if (rRay == 0.0)
    {
        dist = 1000.0;
        color = vec4(0.0, 0.20, 0.40, 1.0);
        return;
    }

    float lr = length(rRay);
    float min_dist = 1000.0;
    vec2 res_tc = vec2(0.0, 0.0);
    bool cube = false;
    vec4 cur_color = vec4(0.0, 0.20, 0.40, 1.0);

    for (int i = 0; i < 10; ++i)
    {
        float k;
        vec2 p;

        intersectK(vertex.position, rRay,
            rect[i].axesOrthIdx, rect[i].axesValue, k, p);

        if (k > 0.0 && inRect(p, rect[i].bound_p1, rect[i].bound_p2))
        {
            vec2 texCoord = (p - rect[i].bound_p1) /
                (rect[i].bound_p2 - rect[i].bound_p1) * rect[i].texScale;
            float d = k * lr;

            if (d < min_dist)
            {
                min_dist = d;
                res_tc = texCoord;
                cube = rect[i].cube;
            }
        }
    }

    if (min_dist < 1000.0)
    {
        if (cube)
        {
            cur_color = texture(texCube, res_tc);
        }
        else if (!cube)
        {
            cur_color = texture(texPool, res_tc);
        }
    }

    dist = min_dist;
    color = cur_color;
}

// tc -- texture coordinates.
vec3 calcNormal(vec2 tc, vec3 to_camera_norm)
{
    float zL = texture(texGeometry, vec2(tc.x - meshTexStep.x, tc.y)).r;
    float zR = texture(texGeometry, vec2(tc.x + meshTexStep.x, tc.y)).r;
    float zD = texture(texGeometry, vec2(tc.x, tc.y - meshTexStep.y)).r;
    float zU = texture(texGeometry, vec2(tc.x, tc.y + meshTexStep.y)).r;

    float zLL = texture(texGeometry, vec2(tc.x - meshTexStep.x * 3.0, tc.y)).r;
    float zRR = texture(texGeometry, vec2(tc.x + meshTexStep.x * 3.0, tc.y)).r;
    float zDD = texture(texGeometry, vec2(tc.x, tc.y - meshTexStep.y * 3.0)).r;
    float zUU = texture(texGeometry, vec2(tc.x, tc.y + meshTexStep.y * 3.0)).r;

    vec3 dx = vec3(pow(2.0 * meshTexStep.x, 2) + 6.0 * meshTexStep.x, 0.0, pow(zR - zL, 2) + (zRR - zLL));
    vec3 dy = vec3(0.0, pow(2.0 * meshTexStep.y, 2) + 6.0 * meshTexStep.y, pow(zU - zD, 2) + (zUU - zDD));

    //vec3 normal = normalize(vec3(zL - zR, zD - zU, 2.0 * meshTexStep.x));
    vec3 normal = normalize(cross(dx, dy));

    if (dot(to_camera_norm, normal) < 0.0)
    {
        normal = -normal;
    }

    return normal;
}

void main(void)
{
    vec3 to_camera = transform.viewPosition - vertex.position;
    vec3 to_camera_norm = normalize(to_camera);

    vec2 texCoord = (vertex.position.xy - meshViewFirst) / meshViewSize;
    vec3 normal = calcNormal(texCoord, to_camera_norm);

    vec4 specular_light = getSpecularLight(to_camera_norm, normal);

    vec4 waterColor = vec4(0.0, 0.20, 0.40, 1.0);

    vec4 refractColor;
    vec4 reflectColor;
    float dist1, dist2;

    if (vertex.under > 0.0)
    {
        vec3 rRay = refract_fork(-to_camera_norm, normal, 1.0 / 1.333);

        calcRayColor(rRay, normal, dist1, refractColor);
        float wf = water_color_factor(dist1);
        refractColor = mix(refractColor, waterColor, wf);

        rRay = reflect(-to_camera_norm, normal);
        calcRayColor(rRay, normal, dist2, reflectColor);
        wf = water_color_factor(dist2);
        reflectColor = mix(reflectColor, waterColor, wf);;
    }
    else
    {
        vec3 rRay = refract_fork(-to_camera_norm, normal, 1.333 / 1.0);

        calcRayColor(rRay, normal, dist1, refractColor);
        float wf = water_color_factor(length(to_camera));
        refractColor = mix(refractColor, waterColor, wf);

        rRay = reflect(-to_camera_norm, normal);
        calcRayColor(rRay, normal, dist2, reflectColor);
        wf = water_color_factor(dist2 + length(to_camera));
        reflectColor = mix(reflectColor, waterColor, wf);
    }

//    float frenel = 1.0 - dot(to_camera_norm, normal);
    float frenel;
//    if (refractColor == waterColor)
//        frenel = 1.0;
//    else
        frenel = 1.0 / pow(1.0 + dot(to_camera_norm, normal), 8.0);

    vec4 rrColor = mix(refractColor, reflectColor, frenel);
    waterColor = rrColor;

    color = (vertex.light + specular_light) * waterColor;
}
