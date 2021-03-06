//*********************************************************
// The MIT License(MIT)
// Copyright(c) 2016 tositeru
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files(the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions :
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//*********************************************************

cbuffer Param : register(b0)
{
	float cbDetailFactor;
	float cbDensityFactor;
};

struct VS_CONTROL_POINT_OUTPUT
{
	float3 pos : POSITION;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float3 pos : POSITION;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[2]	: SV_TessFactor;
};

#define NUM_CONTROL_POINTS 2

// パッチ定数関数
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	Output.EdgeTessFactor[0] = cbDetailFactor;
	Output.EdgeTessFactor[1] = cbDensityFactor;

	float roundedDetail;
	float roundedDensity;
	ProcessIsolineTessFactors(cbDetailFactor, cbDensityFactor, roundedDetail, roundedDensity);

	Output.EdgeTessFactor[0] = roundedDetail;
	Output.EdgeTessFactor[1] = roundedDensity;

	return Output;
}

[domain("isoline")]
[partitioning("pow2")]
[outputtopology("line")]
[outputcontrolpoints(2)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.f)]
HS_CONTROL_POINT_OUTPUT main(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> inputPatch,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONTROL_POINT_OUTPUT Output;
	Output.pos = inputPatch[i].pos;
	return Output;
}
