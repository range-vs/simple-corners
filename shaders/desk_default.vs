cbuffer ConstantBufferOrtho : register(b0)
{
	matrix ortho;
}

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 texcoord: TEXCOORD0;
};

struct VS_INPUT
{
	float2 pos: POSITION;
	float2 texcoord: TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT _out = (VS_OUTPUT)0;
	
	_out.pos = float4(input.pos.x, input.pos.y, 0, 1);
    _out.pos = mul( _out.pos, ortho); 
    
	_out.texcoord = input.texcoord;
	return _out;
}