#version 330 core
out vec4 FragColor;
in vec2 fragCoord;

uniform vec2 iResolution;

float sdf_sphere(vec3 p, float r) {
    return length(p) - r;
}

float sdf_box(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(max(q.x, q.y), q.z), 0.0);
}

float sceneSDF(vec3 p) {
    float d1 = sdf_sphere(p - vec3(0, 0, -3), 1.0);
    float d2 = sdf_box(p - vec3(1.5, 0, -3), vec3(0.8));
    return min(d1, d2); // union
}

vec3 getNormal(vec3 p) {
    float h = 0.001;
    vec2 k = vec2(1, -1);
    return normalize(k.xyy * sceneSDF(p + k.xyy * h) +
                     k.yyx * sceneSDF(p + k.yyx * h) +
                     k.yxy * sceneSDF(p + k.yxy * h) +
                     k.xxx * sceneSDF(p + k.xxx * h));
}

void main() {
    vec2 uv = fragCoord;
    vec2 res = vec2(800.0, 600.0); // hardcoded resolution
    vec2 p = (uv * 0.5 + 0.5) * res;
    vec2 iuv = (gl_FragCoord.xy / res) * 2.0 - 1.0;
    vec3 ro = vec3(0.0, 0.0, 2.0);
    vec3 rd = normalize(vec3(iuv, -1.5));

    float t = 0.0;
    float d;
    for (int i = 0; i < 128; ++i) {
        vec3 pos = ro + t * rd;
        d = sceneSDF(pos);
        if (d < 0.001) break;
        t += d;
        if (t > 20.0) break;
    }

    vec3 col = vec3(0);
    if (d < 0.001) {
        vec3 pos = ro + t * rd;
        vec3 n = getNormal(pos);
        col = 0.5 + 0.5 * n;
    }

    FragColor = vec4(col, 1.0);
}
