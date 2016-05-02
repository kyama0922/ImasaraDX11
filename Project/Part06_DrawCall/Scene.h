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

#pragma once

#include "Template/DXSample.h"
#include <DirectXTK\Inc\SimpleMath.h>

class Scene : public DXSample
{
public:
	Scene(UINT width, UINT height, std::wstring name);

	virtual void onInit()override;
	virtual void onUpdate()override;
	virtual void onRender()override;
	virtual void onDestroy()override;

	virtual void onKeyUp(UINT8 key)override;

private:
	struct Vertex {
		DirectX::SimpleMath::Vector3 pos;
	};

private:
	std::vector<DirectX::SimpleMath::Vector3> makeInstanceData()const;
	void updateTitle();

private:
	enum SHADER_MODE {
		eMODE_DRAW,
		eMODE_DRAW_INDEXED,
		eMODE_DRAW_INSTANCED,
		eMODE_DRAW_INDEXED_INSTANCED,
		eMODE_DRAW_INDEXED_INDIRECT,
		eMODE_COUNT,
	} mMode = eMODE_DRAW;
	static const UINT M_INSTANCED_COUNT = 1000;

	Microsoft::WRL::ComPtr<ID3D11Buffer> mpTriangleBuffer;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> mpPixelShader;

	//Draw, DrawIndexed
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mpInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mpIndexBuffer;

	//���_�X���b�g���g�����C���X�^���X�`��
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShaderByInstanced1;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mpInputLayoutWithSlot2;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mpOffsetBufferBySlot;

	//�\�����o�b�t�@���g�����C���X�^���X�`��
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mpVertexShaderByInstanced2;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mpOffsetBufferByStructured;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mpOffsetBufferByStructuredSRV;
	
	//�C���_�C���N�g
	Microsoft::WRL::ComPtr<ID3D11Buffer> mpIndirectDrawBuffer;
};
