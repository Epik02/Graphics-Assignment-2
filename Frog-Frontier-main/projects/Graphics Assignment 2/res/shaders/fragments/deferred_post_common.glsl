
uniform layout(binding=0) sampler2D s_Depth;
uniform layout(binding=1) sampler2D s_AlbedoSpec;
uniform layout(binding=2) sampler2D s_NormalsMetallic;
uniform layout(binding=3) sampler2D s_Emissive;
uniform layout(binding=4) sampler2D s_Position;


vec3 GetNormal(vec2 uv) {
    return ((texture(s_NormalsMetallic, uv).xyz) * 2) - 1;
}

vec3 GetAlbedo(vec2 uv) {
    return texture(s_AlbedoSpec, uv).rgb;
}

vec3 GetViewPosition(vec2 uv) {
    return texture(s_Position, uv).rgb;
}