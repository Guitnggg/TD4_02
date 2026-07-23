struct PSInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };

float4 main(PSInput input) : SV_TARGET {
    float2 centered = input.uv * 2.0f - 1.0f;
    float radialAlpha = saturate(1.0f - dot(centered, centered));
    return float4(input.color.rgb, input.color.a * radialAlpha);
}
