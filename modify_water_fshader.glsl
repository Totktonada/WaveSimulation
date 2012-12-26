#version 330 core

uniform sampler2D texFst;
uniform sampler2D texSnd;

uniform vec2 meshTexStep;
uniform vec2 meshViewFirst;
uniform vec2 meshViewSize;

uniform bool makeWave;
uniform float meshZ;

uniform struct Transform
{
    mat4 rot;
    vec3 viewPosition;
}
transform;

in vec2 vpos;
out vec4 gl_FragData[1];

const float pi = 3.141592654;

// Is point p in rectangle with left-bottom corner x and
// right-up corner y?
bool inRect(vec2 p, vec2 x, vec2 y)
{
    return (all(greaterThanEqual(p, x)) && all(lessThanEqual(p, y)));
}

// Point of intersect vector "ray" emitted from position "pos" and
// plane with x, y, or z == value (at idx == 0, 1 or 2).
vec3 intersect(vec3 pos, vec3 ray, int idx, float value)
{
    float k = (value - pos[idx]) / ray[idx];
    return pos + ray * k;
}

float calcZ(vec2 xy)
{
    vec2 tc = (xy - meshViewFirst) / meshViewSize;
    float z = texture(texSnd, tc).r;

    return z;
}

void main(void)
{
    float texX = (vpos.x*10 - meshViewFirst.x) / meshViewSize.x;
    float texY = (vpos.y*10 - meshViewFirst.y) / meshViewSize.y;

    float zFst = texture(texFst, vec2(texX, texY)).r;
//    float zSnd = texture(texSnd, vec2(texX, texY)).r;

    float zSndL = texture(texSnd, vec2(texX - meshTexStep.x, texY)).r;
    float zSndR = texture(texSnd, vec2(texX + meshTexStep.x, texY)).r;
    float zSndD = texture(texSnd, vec2(texX, texY - meshTexStep.y)).r;
    float zSndU = texture(texSnd, vec2(texX, texY + meshTexStep.y)).r;

    const float w = 1.950;

    float zDst = (1.0 - w) * zFst +
        w * (zSndL + zSndR + zSndD + zSndU) / 4.0;

    if (makeWave)
    {
        float wUnder = sign(transform.viewPosition.z -
            calcZ(transform.viewPosition.xy));

        vec3 viewRay = vec3(transform.rot * vec4(0.0, 0.0, 1.0, 1.0));
        vec3 p = intersect(transform.viewPosition, viewRay, 2, meshZ);

        if (sign(transform.viewPosition.z - meshZ) == sign(viewRay.z) &&
            inRect(p.xy, vec2(-10.0, -10.0), vec2(10.0, 10.0)))
        {
            vec2 texCoord = (p.xy - meshViewFirst) / meshViewSize;
            float dist = distance(vec2(texX, texY), texCoord);

            // Radius of wave.
            float r = 4 * meshTexStep;

            // Full height from dow to up wave.
            float h = 0.4 * wUnder;

            // volume of water cast down and cast up
            // equaled by substrict "c";
            const float c = 0.3600349828087051;

            if (dist <= r)
            {
                dist = dist / r * (pi / 2);
                zDst -= h * (cos(dist) - c);
            }
        }
    }

    gl_FragData[0] = vec4(zDst, 0.0, 0.0, 0.0);
}
