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

struct Dummy {};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 color : TEXCOORD0;
	uint arrayIndex : SV_RenderTargetArrayIndex;
};

static const float4 gColorTable[6] = {
	float4(1.0f, 0.7f, 0.7f, 1),
	float4(0.7f, 1.0f, 0.7f, 1),
	float4(0.7f, 0.7f, 1.0f, 1),
	float4(1.0f, 1.0f, 0.7f, 1),
	float4(1.0f, 0.7f, 1.0f, 1),
	float4(0.7f, 1.0f, 1.0f, 1),
};

static const float4 gPosTable[3] = {
	float4( 0.0f, 0.51f, 0.0f, 1.0f),
	float4( 0.51f,-0.52f, 0.0f, 1.0f),
	float4(-0.53f,-0.53f, 0.0f, 1.0f),
};

[maxvertexcount(3 * 6)]
void main(
	point Dummy input[1], 
	inout TriangleStream< GSOutput > output
)
{
	//６面分の三角形を生成している
	[unroll] for (uint n = 0; n < 6; ++n) {
		[unroll] for (uint i = 0; i < 3; i++) {
			GSOutput element;
			element.pos = gPosTable[i];
			element.color = gColorTable[n];
			//どの要素に書き込むか指定している
			element.arrayIndex = n;
			output.Append(element);
		}
		output.RestartStrip();
	}
}