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

#include "Scene.h"

#include <random>

Scene::Scene(UINT width, UINT height, std::wstring name)
	: DXSample(width, height, name)
{
}

void Scene::onInit()
{
	{//�O���t�B�b�N�X�p�C�v���C���̏�����
		std::vector<char> byteCode;
		createShader(this->mpVertexShader.GetAddressOf(), this->mpDevice.Get(), "VS.cso", &byteCode);

		//���̓��C�A�E�g�̍쐬
		std::array<D3D11_INPUT_ELEMENT_DESC, 1> elements = { {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		} };
		auto hr = this->mpDevice->CreateInputLayout(elements.data(), static_cast<UINT>(elements.size()), byteCode.data(), byteCode.size(), this->mpInputLayout.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("���̓��C�A�E�g�̍쐬�Ɏ��s");
		}
		//�s�N�Z���V�F�[�_
		createShader(this->mpPixelShader.GetAddressOf(), this->mpDevice.Get(), "PixelShader.cso", &byteCode);

		//�O�p�`�̃e�b�Z���[�V�����V�F�[�_
		createShader(this->mpHSTriangle.GetAddressOf(), this->mpDevice.Get(), "HSTriangle.cso");
		createShader(this->mpDSTriangle.GetAddressOf(), this->mpDevice.Get(), "DSTriangle.cso");

		//�l�p�`�̃e�b�Z���[�V�����V�F�[�_
		createShader(this->mpHSQuad.GetAddressOf(), this->mpDevice.Get(), "HSQuad.cso");
		createShader(this->mpDSQuad.GetAddressOf(), this->mpDevice.Get(), "DSQuad.cso");

		//�����̃e�b�Z���[�V�����V�F�[�_
		createShader(this->mpHSIsoline.GetAddressOf(), this->mpDevice.Get(), "HSIsoline.cso");
		createShader(this->mpDSIsoline.GetAddressOf(), this->mpDevice.Get(), "DSIsoline.cso");

		//�_����O�p�`�𐶐�����e�b�Z���[�V�����V�F�[�_
		createShader(this->mpHSPointToTriangle.GetAddressOf(), this->mpDevice.Get(), "HSPointToTriangle.cso");
		createShader(this->mpDSPointToTriangle.GetAddressOf(), this->mpDevice.Get(), "DSPointToTriangle.cso");
	}
	{//
		std::array<DirectX::SimpleMath::Vector3, 9> data = { {
			//�O�p�`
			{ 0.0f, 0.5f, 0.0f },
			{-0.5f,-0.5f, 0.0f },
			{ 0.5f,-0.5f, 0.0f },

			//����
			{-0.5f, 0.0f, 0.0f },
			{ 0.5f, 0.0f, 0.0f },

			//�l�p�`
			{-0.5f, 0.5f, 0.0f },
			{ 0.5f, 0.5f, 0.0f },
			{-0.5f,-0.5f, 0.0f },
			{ 0.5f,-0.5f, 0.0f },
		} };
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = static_cast<UINT>(sizeof(data[0]) * data.size());
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data.data();
		initData.SysMemPitch = desc.ByteWidth;
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpVertexBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("���_�o�b�t�@�̍쐬�Ɏ��s");
		}
	}
	{//�n���V�F�[�_�p�̒萔�o�b�t�@�쐬
		this->mHSParam.edgeFactor = 4.f;
		this->mHSParam.insideFactor = 2.f;

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(this->mHSParam);
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &this->mHSParam;
		initData.SysMemPitch = desc.ByteWidth;
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpHSParamBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�n���V�F�[�_�p�̒萔�o�b�t�@�쐬�Ɏ��s");
		}
	}
	{//
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_WIREFRAME;
		//desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.AntialiasedLineEnable = true;
		auto hr = this->mpDevice->CreateRasterizerState(&desc, this->mpRSWireframe.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("���X�^���C�Y�X�e�[�g�̍쐬�Ɏ��s");
		}
	}
}

void Scene::onUpdate()
{
}

void Scene::onKeyDown(UINT8 key)
{
	if (key == VK_LEFT) {
		this->mHSParam.edgeFactor = max(0, this->mHSParam.edgeFactor - 0.5f);
	}
	if (key == VK_RIGHT) {
		this->mHSParam.edgeFactor = min(64.f, this->mHSParam.edgeFactor + 0.5f);
	}
	if (key == VK_UP) {
		this->mHSParam.insideFactor = max(0, this->mHSParam.insideFactor + 0.5f);
	}
	if (key == VK_DOWN) {
		this->mHSParam.insideFactor = min(64.f, this->mHSParam.insideFactor - 0.5f);
	}
}

void Scene::onKeyUp(UINT8 key)
{
	if (key == 'Z') {
		this->mMode = static_cast<decltype(this->mMode)>((this->mMode + 1) % eMODE_COUNT);
	}
}

void Scene::onRender()
{
	this->mpImmediateContext->UpdateSubresource(this->mpHSParamBuffer.Get(), 0, nullptr, &this->mHSParam, sizeof(this->mHSParam), sizeof(this->mHSParam));

	UINT offset = 0;
	UINT drawVertexCount = 0;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	switch (this->mMode) {
	case eMODE_TRIANGLE:
		offset = 0;
		drawVertexCount = 3;
		topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

		//�n���V�F�[�_
		this->mpImmediateContext->HSSetShader(this->mpHSTriangle.Get(), nullptr, 0);
		//�h���C���V�F�[�_
		this->mpImmediateContext->DSSetShader(this->mpDSTriangle.Get(), nullptr, 0);
		break;
	case eMODE_QUAD:
		offset = 5;
		drawVertexCount = 4;
		topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

		//�n���V�F�[�_
		this->mpImmediateContext->HSSetShader(this->mpHSQuad.Get(), nullptr, 0);
		//�h���C���V�F�[�_
		this->mpImmediateContext->DSSetShader(this->mpDSQuad.Get(), nullptr, 0);
		break;
	case eMODE_ISOLINE:
		offset = 3;
		drawVertexCount = 2;
		topology = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;

		//�n���V�F�[�_
		this->mpImmediateContext->HSSetShader(this->mpHSIsoline.Get(), nullptr, 0);
		//�h���C���V�F�[�_
		this->mpImmediateContext->DSSetShader(this->mpDSIsoline.Get(), nullptr, 0);
		break;
	case eMODE_POINT_TO_TRIANGLE:
		offset = 0;
		drawVertexCount = 1;
		topology = D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;

		//�n���V�F�[�_
		this->mpImmediateContext->HSSetShader(this->mpHSPointToTriangle.Get(), nullptr, 0);
		//�h���C���V�F�[�_
		this->mpImmediateContext->DSSetShader(this->mpDSPointToTriangle.Get(), nullptr, 0);
		break;
	default:
		assert(false);
	}
	//offset = 0;

	//���̓A�Z���u���X�e�[�W
	std::array<ID3D11Buffer*, 1> ppBufs = { { this->mpVertexBuffer.Get() } };
	std::array<UINT, 1> pStrides = { { sizeof(DirectX::SimpleMath::Vector3) } };
	std::array<UINT, 1> pOffsets = { { offset * sizeof(DirectX::SimpleMath::Vector3) } };
	this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppBufs.size()), ppBufs.data(), pStrides.data(), pOffsets.data());
	this->mpImmediateContext->IASetPrimitiveTopology(topology);
	this->mpImmediateContext->IASetInputLayout(this->mpInputLayout.Get());

	//���_�V�F�[�_
	this->mpImmediateContext->VSSetShader(this->mpVertexShader.Get(), nullptr, 0);
	//�n���V�F�[�_
	std::array<ID3D11Buffer*, 1> ppCBs = { { this->mpHSParamBuffer.Get() } };
	this->mpImmediateContext->HSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());

	//���X�^���C�U�[�X�e�[�g
	this->mpImmediateContext->RSSetState(this->mpRSWireframe.Get());
	//�s�N�Z���V�F�[�_
	this->mpImmediateContext->PSSetShader(this->mpPixelShader.Get(), nullptr, 0);

	this->mpImmediateContext->Draw(drawVertexCount, 0);
}

void Scene::onDestroy()
{
}
