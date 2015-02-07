struct VertexInput {
	float4 position : POSITION;
	float2 TexY	: TEXCOORD0;
	float2 TexU	: TEXCOORD1;
	float2 TexV	: TEXCOORD2;
};

struct PixelInput{
	float4 position : SV_POSITION;
	float2 TexY	: TEXCOORD0;
	float2 TexU	: TEXCOORD1;
	float2 TexV	: TEXCOORD2;
};

cbuffer cbPerObject
{
	float4x4 gW;
	float4x4 gV;
	float4x4 gP;
	float4x4 g_ColorMatrix;
	float2  g_StepXY;
};

Texture2D g_YTexture;
Texture2D g_UTexture;
Texture2D g_VTexture;

PixelInput VS(VertexInput input) {
	PixelInput output = (PixelInput)0;
	output.position = mul(input.position, gW);
	output.position = mul(output.position, gV);
	output.position = mul(output.position, gP);
	output.TexY = input.TexY;
	output.TexU = input.TexU;
	output.TexV = input.TexV;

	return output;
}

SamplerState YSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState USampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState VSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

/*sampler USampler = sampler_state {
	Texture = <g_UTexture>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};
sampler VSampler = sampler_state
{
	Texture = <g_VTexture>;
	AddressU = CLAMP;
	AddressV = CLAMP;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};*/

float4 PS(PixelInput input) : SV_Target {

#if defined(XBMC_YV12)
	float4 YUV = float4(g_YTexture.Sample(YSampler, input.TexY).r, 
						g_UTexture.Sample(USampler, input.TexU).r, 
						g_VTexture.Sample(VSampler, input.TexV).r, 
						1.0);
#elif defined(XBMC_NV12)
	float4 YUV = float4(g_YTexture.Sample(YSampler, input.TexY).r,
						g_UTexture.Sample(USampler, input.TexU).rg,
						1.0);
#elif defined(XBMC_YUY2) || defined(XBMC_UYVY)
	// The HLSL compiler is smart enough to optimize away these redundant assignments.
	// That way the code is almost identical to the OGL shader.
	float2 stepxy = g_StepXY;
	float2 pos = input.TexY;
		pos = float2(pos.x - (stepxy.x * 0.25), pos.y);
	float2 f = frac(pos / stepxy);

		//y axis will be correctly interpolated by opengl
		//x axis will not, so we grab two pixels at the center of two columns and interpolate ourselves
		float4 c1 = g_YTexture.Sample(YSampler, float2(pos.x + ((0.5 - f.x) * stepxy.x), pos.y));
		float4 c2 = g_YTexture.Sample(YSampler, float2(pos.x + ((1.5 - f.x) * stepxy.x), pos.y));

		/* each pixel has two Y subpixels and one UV subpixel
		YUV  Y  YUV
		check if we're left or right of the middle Y subpixel and interpolate accordingly*/
#if defined(XBMC_YUY2) // BGRA = YUYV
		float  leftY = lerp(c1.b, c1.r, f.x * 2.0);
	float  rightY = lerp(c1.r, c2.b, f.x * 2.0 - 1.0);
	float2 outUV = lerp(c1.ga, c2.ga, f.x);
#elif defined(XBMC_UYVY) // BGRA = UYVY
		float  leftY = lerp(c1.g, c1.a, f.x * 2.0);
	float  rightY = lerp(c1.a, c2.g, f.x * 2.0 - 1.0);
	float2 outUV = lerp(c1.br, c2.br, f.x);
#endif
		float  outY = lerp(leftY, rightY, step(0.5, f.x));
	float4 YUV = float4(outY, outUV, 1.0);
#endif

		float4 RGBColor = mul(YUV, g_ColorMatrix);
			RGBColor.a = 1.0;
	return RGBColor;
}

technique10 YUV2RGB {
	pass P0 {
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
};

