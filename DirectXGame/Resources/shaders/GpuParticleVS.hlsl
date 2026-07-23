#pragma pack_matrix(row_major)

cbuffer ViewProjection : register(b0) {
    matrix view;
    matrix projection;
};

struct Particle {
    float3 position;
    float scale;
    float3 velocity;
    float life;
    float age;
    float startScale;
    float endScale;
    float active;
    float4 startColor;
    float4 endColor;
};

StructuredBuffer<Particle> particles : register(t0);

struct VSInput { float2 position : POSITION; uint instanceId : SV_InstanceID; };
struct VSOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; float4 color : COLOR0; };

VSOutput main(VSInput input) {
    VSOutput output;
    Particle particle = particles[input.instanceId];
    float t = particle.life > 0.0f ? saturate(particle.age / particle.life) : 1.0f;
    float visible = particle.active > 0.5f ? 1.0f : 0.0f;
    float4 viewPosition = mul(float4(particle.position, 1.0f), view);
    viewPosition.xy += input.position * particle.scale * visible;
    output.position = mul(viewPosition, projection);
    output.uv = input.position + 0.5f;
    output.color = lerp(particle.startColor, particle.endColor, t);
    output.color.a *= visible;
    return output;
}
