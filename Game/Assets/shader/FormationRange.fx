/**
 * @file FormationRange.fx
 * @brief 陣形の入隊・脱隊範囲を地形追従ラインで描画するシェーダー
 */

cbuffer VSCb : register(b0)
{
    float4x4 mVP;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(mVP, float4(input.position, 1.0f));
    output.color    = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
