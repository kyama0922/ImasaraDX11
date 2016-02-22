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
	this->createScreenTexture();

	struct SphereInfo {
		DirectX::SimpleMath::Vector3 pos;
		float range;
		DirectX::SimpleMath::Vector3 color;
		float pad;//�\���̂̃T�C�Y��16�̔{���ɂ��邽�߂ɐ錾���Ă���
	};

	{//�萔�o�b�t�@���g�������̕`��̏�����
		this->compileComputeShader(&this->mpCSRenderSphereByCB, "RenderSphereByConstantBuffer.cso");

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(SphereInfo);
		desc.Usage = D3D11_USAGE_DEFAULT;

		SphereInfo info;
		info.pos = DirectX::SimpleMath::Vector3(0, 0, 30.f);
		info.range = 10.f;
		info.color = DirectX::SimpleMath::Vector3(1, 1, 0.4f);
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &info;
		initData.SysMemPitch = sizeof(info);
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpConstantBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�萔�o�b�t�@�̍쐬�Ɏ��s");
		}
	}


	{//�J�����֌W�̒萔�o�b�t�@�̍쐬
		DirectX::SimpleMath::Matrix invPerspective;
		invPerspective = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(tan(45.f * DirectX::XM_PI / 180.f), this->mWidth /static_cast<float>(this->mHeight), 0.1f, 100.f).Invert().Transpose();

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(invPerspective);
		desc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &invPerspective;
		initData.SysMemPitch = sizeof(invPerspective);
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpCBCamera.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�J�����p�萔�o�b�t�@�̍쐬�Ɏ��s");
		}
	}
}

void Scene::onUpdate()
{
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
	//switch (this->mMode) {
	//case eMODE_INDEX:	this->mpImmediateContext->CSSetShader(this->mpCSClearScreen.Get(), nullptr, 0); break;
	//case eMODE_SAMPLER:	this->mpImmediateContext->CSSetShader(this->mpCSClearScreenWithSampler.Get(), nullptr, 0); break;
	//default:
	//	assert(false);
	//}
	this->mpImmediateContext->CSSetShader(this->mpCSRenderSphereByCB.Get(), nullptr, 0);
	std::array<ID3D11Buffer*, 2> ppCBs = { { 
			this->mpCBCamera.Get(),
			this->mpConstantBuffer.Get(), 
	} };
	this->mpImmediateContext->CSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());

	//std::array<ID3D11ShaderResourceView*, 1> ppSRVs = { { this->mpImageSRV.Get(), } };
	//this->mpImmediateContext->CSSetShaderResources(0, static_cast<UINT>(ppSRVs.size()), ppSRVs.data());

	std::array<ID3D11UnorderedAccessView*, 1> ppUAVs = { { this->mpScreenUAV.Get(), } };
	std::array<UINT, 1> initCounts = { { 0u, } };
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), initCounts.data());

	//���s
	this->mpImmediateContext->Dispatch(this->mWidth, this->mHeight, 1);


	//�V�F�[�_�̌��ʂ��o�b�N�o�b�t�@�[�ɃR�s�[����
	this->mpImmediateContext->CopySubresourceRegion(this->mpBackBuffer.Get(), 0, 0, 0, 0, this->mpScreen.Get(), 0, nullptr);
}

void Scene::onDestroy()
{
}



void Scene::compileComputeShader(Microsoft::WRL::ComPtr<ID3D11ComputeShader>* pOut, const std::string& filepath)
{
	std::vector<char> byteCode;
	if (!loadBinaryFile(&byteCode, filepath.c_str())) {
		throw std::runtime_error(filepath + "�̓ǂݍ��݂Ɏ��s");
	}

	HRESULT hr;
	hr = this->mpDevice->CreateComputeShader(byteCode.data(), static_cast<SIZE_T>(byteCode.size()), nullptr, pOut->GetAddressOf());
	if (FAILED(hr)) {
		throw std::runtime_error(filepath + "�̍쐬�Ɏ��s");
	}
}

void Scene::createScreenTexture()
{
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
