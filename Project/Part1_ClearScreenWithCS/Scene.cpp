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

#include <DirectXTK\Inc\SimpleMath.h>

Scene::Scene(UINT width, UINT height, std::wstring name)
	: DXSample(width, height, name)
{
}

void Scene::onInit()
{
	{
		std::vector<char> byteCode;
		if (!loadBinaryFile(&byteCode, "ClearScreen.cso")) {
			throw std::runtime_error("�V�F�[�_�t�@�C���̓ǂݍ��݂Ɏ��s");
		}

		HRESULT hr;
		hr = this->mpDevice->CreateComputeShader(byteCode.data(), static_cast<SIZE_T>(byteCode.size()), nullptr, &this->mpCSClearScreen);
		if (FAILED(hr)) {
			throw std::runtime_error("ID3D11ComputerShader�̍쐬�Ɏ��s");
		}

		if (!loadBinaryFile(&byteCode, "ClearScreenWithConstantBuffer.cso")) {
			throw std::runtime_error("�V�F�[�_�t�@�C���̓ǂݍ��݂Ɏ��s");
		}
		hr = this->mpDevice->CreateComputeShader(byteCode.data(), static_cast<SIZE_T>(byteCode.size()), nullptr, &this->mpCSClearScreenWithConstantBuffer);
		if (FAILED(hr)) {
			throw std::runtime_error("ClearScreenWithConstantBuffer.cso�̍쐬�Ɏ��s");
		}
	}

	{//��ʃN���A�p�̒萔�o�b�t�@�̍쐬
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(float) * 4;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;

		auto hr = this->mpDevice->CreateBuffer(&desc, nullptr, this->mpClearColor.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("��ʃN���A�[�p�̒萔�o�b�t�@�̍쐬�Ɏ��s");
		}
	}
	{//ClearScreen.hlsl�̏o�͐�̍쐬
		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = this->mWidth;
		desc.Height = this->mHeight;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		auto hr = this->mpDevice->CreateTexture2D(&desc, nullptr, &this->mpScreen);
		if (FAILED(hr)) {
			throw std::runtime_error("�R���s���[�^�V�F�[�_�̏o�͐�p��ID3D11Texture2D�̍쐬�Ɏ��s");
		}

		hr = this->mpDevice->CreateUnorderedAccessView(this->mpScreen.Get(), nullptr, &this->mpScreenUAV);
		if (FAILED(hr)) {
			throw std::runtime_error("�R���s���[�^�V�F�[�_�̏o�͐�p��ID3D11Texture2D��UnorderedAccessView�̍쐬�Ɏ��s");
		}
	}
}

void Scene::onUpdate()
{
	static float t = 0.f;
	t += 0.01f;
	if (1 < t) { t = 0.f; }

	DirectX::SimpleMath::Vector4 setValue(1, t, 0, 1);
	this->mpImmediateContext->UpdateSubresource(this->mpClearColor.Get(), 0, nullptr, &setValue, sizeof(setValue), sizeof(setValue));
	//todo Map/Unmap�ɂ��Đ�������H
}

void Scene::onKeyUp(UINT8 key)
{
	if (key == 'Z') {
		this->mMode = static_cast<decltype(this->mMode)>((this->mMode + 1) % eMODE_COUNT);
	}
}


void Scene::onRender()
{
	//GPU�ɕK�v�ȃf�[�^��ݒ肷��
	switch (this->mMode) {
	case eMODE_SOLID:					this->mpImmediateContext->CSSetShader(this->mpCSClearScreen.Get(), nullptr, 0); break;
	case eMODE_WITH_CONSTANT_BUFFER:	this->mpImmediateContext->CSSetShader(this->mpCSClearScreenWithConstantBuffer.Get(), nullptr, 0); break;
	default:
		assert(false);
	}

	std::array<ID3D11Buffer*, 1> ppCBs = { { this->mpClearColor.Get(), } };
	this->mpImmediateContext->CSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());

	std::array<ID3D11UnorderedAccessView*, 1> ppUAVs = { { this->mpScreenUAV.Get(), } };
	std::array<UINT, 1> initCounts = { { 0u, } };
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), initCounts.data());

	//ClearScreen.hlsl�̎��s
	this->mpImmediateContext->Dispatch(this->mWidth, this->mHeight, 1);


	//�V�F�[�_�̌��ʂ��o�b�N�o�b�t�@�[�ɃR�s�[����
	this->mpImmediateContext->CopySubresourceRegion(this->mpBackBuffer.Get(), 0, 0, 0, 0, this->mpScreen.Get(), 0, nullptr);
}

void Scene::onDestroy()
{
}

