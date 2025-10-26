#version 330 core

in vec3 worldPos;
out vec4 FragColor;

uniform vec3 gridColor = vec3(0.55);
uniform float majorScale = 10.0;
uniform float lineThickness = 3.0;
uniform vec3 cameraPos;

// AA grid function
float grid(vec2 coord) {
    vec2 g = abs(fract(coord - 0.5) - 0.5) / (fwidth(coord) * lineThickness);
    float line = min(g.x, g.y);
    return 1.0 - clamp(line, 0.0, 1.0);
}

void main() {
    // XY plane coordinates for grid
    vec2 coord = worldPos.xy;

    float major = grid(coord / majorScale);
    float minor = grid(coord);

    float intensity = max(minor * 0.8, major);

    // Fade into distance to look infinite
    float dist = length(coord - cameraPos.xy);
    float fade = clamp(1.0 - dist * 0.002, 0.0, 1.0);

    float alpha = intensity * fade * 0.75; // 0.75 → tweakable

    FragColor = vec4(gridColor * intensity * fade, alpha);
}