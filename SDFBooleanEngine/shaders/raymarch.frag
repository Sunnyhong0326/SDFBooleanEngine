#version 430 core

out vec4 FragColor;
in vec2 fragCoord;

uniform mat4 invViewProj;
uniform vec2 iResolution;
uniform vec3 camPos;

#define MAX_LIGHTS 4

uniform int numLights;
uniform vec3 lights[MAX_LIGHTS]; // light directions

// Node type constants
#define SPHERE    0
#define BOX       1
#define PLANE     2
#define TRI_PRISM 3
#define UNION     10
#define INTERSECT 11
#define SUBTRACT  12

// Struct definition matches std430 layout: 3x vec4 = 48 bytes
struct SDFNode {
    int type;
    int left;
    int right;   // x = type, y = left, z = right
    int pad0;
    vec4 param1;  // xyz = center or size
    vec4 param2;  // x = radius (for sphere or box half-extent)
    vec3 color;
    float pad;
};

layout(std430, binding = 0) buffer SDFBuffer {
    SDFNode nodes[];
};

int hitIndex = -1;

struct EvalResult {
    float dist;
    int hitId;
};

float sdTriPrism(vec3 p, vec2 h) {
    vec3 q = abs(p);
    return max(q.z - h.y, max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5);
}

float sdSphere(vec3 p, float s)
{
  return length(p) - s;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

// Root scene SDF
float sceneSDF(vec3 p) {
    const int MAX_STACK = 64;


    EvalResult resultStack[MAX_STACK];
    int rs = 0;

    int nodeStack[MAX_STACK];
    int ns = 0;

    if (nodes.length() == 0)
        return 1e9;

    nodeStack[ns++] = nodes.length() - 1;

    while (ns > 0) {
        int index = nodeStack[--ns];

        if (index < 0) {
            // operator marker
            int opIndex = -index - 1;
            SDFNode opNode = nodes[opIndex];
            int opType = opNode.type;

            EvalResult b = resultStack[--rs];
            EvalResult a = resultStack[--rs];

            EvalResult r;
            if (opType == UNION) {
                r = (a.dist < b.dist) ? a : b;
            } else if (opType == INTERSECT) {
                r.dist = max(a.dist, b.dist);
                r.hitId = (a.dist > b.dist) ? a.hitId : b.hitId;
            } else if (opType == SUBTRACT) {
                r.dist = max(a.dist, -b.dist);
                r.hitId = (a.dist > -b.dist) ? a.hitId : b.hitId;
            }

            resultStack[rs++] = r;
            continue;
        }
        else {
            SDFNode node = nodes[index];
            int type = node.type;
            int left = node.left;
            int right = node.right;

            if (type == SPHERE) {
                vec3 localP = p - node.param1.xyz;
                float d = sdSphere(localP, node.param2.x);
                resultStack[rs++] = EvalResult(d, index);
            } else if (type == BOX) {
                vec3 localP = p - node.param1.xyz;
                float d = sdBox(localP, node.param2.xyz);
                resultStack[rs++] = EvalResult(d, index);
            } else if (type == PLANE) {
                float d = dot(abs(p - node.param1.xyz), node.param2.xyz) + node.param2.w;
                resultStack[rs++] = EvalResult(d, index);
            } else if (type == TRI_PRISM) {
                vec3 localP = p - node.param1.xyz;
                float d = sdTriPrism(localP, vec2(node.param2.x, node.param2.y));
                resultStack[rs++] = EvalResult(d, index);
            } else {
                // Push operator marker and children
                if (ns + 3 <= MAX_STACK) {
                    nodeStack[ns++] = -index - 1; 
                    nodeStack[ns++] = right;
                    nodeStack[ns++] = left;
                }
            }
        }
    }

    if (rs == 0)
        return 1e9;

    EvalResult final = resultStack[--rs];
    hitIndex = final.hitId;
    return final.dist;
}

// Normal approximation
vec3 getNormal(vec3 p) {
    float h = 0.001;
    vec2 k = vec2(1, -1);
    return normalize(
        k.xyy * sceneSDF(p + k.xyy * h) +
        k.yyx * sceneSDF(p + k.yyx * h) +
        k.yxy * sceneSDF(p + k.yxy * h) +
        k.xxx * sceneSDF(p + k.xxx * h)
    );
}

void main() {
    vec2 uv = (gl_FragCoord.xy / iResolution) * 2.0 - 1.0;
    vec4 ndc = vec4(uv, -1.0, 1.0);
    vec4 world = invViewProj * ndc;
    world /= world.w;

    vec3 ro = camPos;
    vec3 rd = normalize(world.xyz - camPos);

    float t = 0.0;
    float d = 0.0;
    const int maxSteps = 128;
    const float epsilon = 0.001;
    const float maxDist = 100.0;

    for (int i = 0; i < maxSteps; ++i) {
        vec3 p = ro + t * rd;
        d = sceneSDF(p);
        if (d < epsilon) break;
        t += d;
        if (t > maxDist) break;
    }

    vec3 color = vec3(0.0);
    if (d < epsilon) {
        vec3 hit = ro + t * rd;
        vec3 normal = getNormal(hit);
        vec3 lightDir = normalize(vec3(0.5, 0.6, 1.0));
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 baseColor = nodes[hitIndex].color; // Replace with your object's color if needed
        vec3 diffuse = vec3(0.0);

        for (int i = 0; i < numLights; ++i) {
            vec3 lightDir = normalize(lights[i]);
            float diff = max(dot(normal, lightDir), 0.0);
            diffuse += diff * baseColor * 2;
        }

        color = diffuse / float(numLights);
        // color = vec3(1.0, 0.2235, 0.2235) * diff; // basic diffuse
    }

    FragColor = vec4(color, 1.0);
}