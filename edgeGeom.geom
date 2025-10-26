#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vertWorldPos[];
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewportSize;
uniform float lineWidth = 1.0;

void main()
{
    // Project endpoints
    vec4 clip0 = projection * view * vec4(vertWorldPos[0], 1.0);
    vec4 clip1 = projection * view * vec4(vertWorldPos[1], 1.0);

    // Convert to NDC
    vec2 ndc0 = clip0.xy / clip0.w;
    vec2 ndc1 = clip1.xy / clip1.w;
    
    // Convert to screen pixels
    vec2 pix0 = (ndc0 * 0.5 + 0.5) * viewportSize;
    vec2 pix1 = (ndc1 * 0.5 + 0.5) * viewportSize;

    vec2 delta = pix1 - pix0;
    float len = length(delta);
    if (len < 1e-6) 
        delta = vec2(1.0, 0.0), len = 1.0;

    vec2 dir = delta / len;
    vec2 normal = vec2(-dir.y, dir.x) * (lineWidth * 0.5);

    vec2 quadPix[4];
    quadPix[0] = pix0 + normal;
    quadPix[1] = pix0 - normal;
    quadPix[2] = pix1 + normal;
    quadPix[3] = pix1 - normal;

    // Emit triangle strip
    for(int i = 0; i < 4; i++)
    {
        float w = (i < 2) ? clip0.w : clip1.w;
        float z = (i < 2) ? clip0.z : clip1.z;

        vec2 ndc = (quadPix[i] / viewportSize) * 2.0 - 1.0;
        gl_Position = vec4(ndc * w, z, w);

        EmitVertex();
    }
    EndPrimitive();
}