struct VertexInput {
	float4 position : POSITION;
	float4 color	: COLOR0;
};

struct PixelInput{
	float4 position : SV_POSITION;
	float4 color	: COLOR0;
};

cbuffer cbPerObject
{
	float4x4 gW;
	float4x4 gV;
	float4x4 gP;
};

PixelInput VS(VertexInput input) {
	PixelInput output = (PixelInput)0;
	output.position = mul(input.position, gW);
	output.position = mul(output.position, gV);
	output.position = mul(output.position, gP);
	output.color = input.color;

	return output;
}

float4 PS(PixelInput input) : SV_Target {
	return input.color;
}

technique10 ColorTech {
	pass P0 {
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
};