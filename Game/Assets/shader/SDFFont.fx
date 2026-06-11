// SDF (Signed Distance Field) font pixel shader.
// Works with SpriteBatch's built-in vertex shader output.
// fwidth() automatically adapts edge sharpness to the current scale,
// so text stays crisp at any size.

Texture2D<float4> Texture : register(t0);
SamplerState LinearSampler : register(s0);

struct PSInput
{
    float4 color    : COLOR0;
    float2 texCoord : TEXCOORD0;
};

float4 PSMain(PSInput input) : SV_Target0
{
    float dist  = Texture.Sample(LinearSampler, input.texCoord).r;
    float width = fwidth(dist) * 0.7f;
    float alpha = smoothstep(0.5f - width, 0.5f + width, dist);
    return float4(input.color.rgb, input.color.a * alpha);
}
