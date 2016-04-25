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

#include <DirectXTK\Inc\WICTextureLoader.h>

#include "Template/Common.h"

Scene::Scene(UINT width, UINT height, std::wstring name)
	: DXSample(width, height, name)
{
}

void Scene::onInit()
{
	this->updateTitle();

	{
		createShader(this->mpCSClearScreen.GetAddressOf(), this->mpDevice.Get(), "ClearScreenWithTexture.cso");

		createShader(this->mpCSClearScreenWithSampler.GetAddressOf(), this->mpDevice.Get(), "ClearScreenWithSampler.cso");
	}
	{//�v���O������Ńf�[�^��p�ӂ��ăe�N�X�`�����쐬����
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = 256;
		desc.Height = 128;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;

		std::vector<uint32_t> rawData;
		rawData.resize(desc.Width * desc.Height);
		for (auto y = 0u; y < desc.Height; ++y) {
			for (auto x = 0u; x < desc.Width; ++x) {
				auto index = y * desc.Width + x;
				if (index % 52 < 10) {
					rawData[index] = 0xff55ffff;
				} else {
					rawData[index] = 0xffff5555;
				}
			}
		}
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = rawData.data();
		initData.SysMemPitch = sizeof(rawData[0]) * desc.Width;//1�s������̃f�[�^��
		initData.SysMemSlicePitch = sizeof(rawData[0]) * desc.Width * desc.Height;//�����ł͑S�̂̃T�C�Y
		auto hr = this->mpDevice->CreateTexture2D(&desc, &initData, this->mpTex2D.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("ID3D11Texture�̍쐬�Ɏ��s");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Format = desc.Format;
		viewDesc.Texture2D.MipLevels = desc.MipLevels;
		viewDesc.Texture2D.MostDetailedMip = 0;
		hr = this->mpDevice->CreateShaderResourceView(this->mpTex2D.Get(), &viewDesc, this->mpTex2DSRV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("ID3D11ShaderResourceView�̍쐬�Ɏ��s");
		}
	}

	{//DirectXTK���g�����摜�ɓǂݍ���
		Microsoft::WRL::ComPtr<ID3D11Resource> pTex2D;
		auto hr = DirectX::CreateWICTextureFromFile(this->mpDevice.Get(), L"image.png", &pTex2D, this->mpImageSRV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("��ʃN���A�p�̉摜�̓ǂݍ��݂Ɏ��s");
		}
		hr = pTex2D.Get()->QueryInterface<ID3D11Texture2D>(this->mpImage.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("ID3D11Resource����ID3D11Texture2D�ւ̕ϊ��Ɏ��s");
		}
	}
	{//�T���v���[�쐬
		D3D11_SAMPLER_DESC desc = {};
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		//desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; //��Ԃ��������T���v�����O
		//todo �~�b�v�}�b�v�̐���������H
		auto hr = this->mpDevice->CreateSamplerState(&desc, this->mpSampler.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�|�C���g�T���v���[�̍쐬�Ɏ��s");
		}
	}

	{//�o�͐�̍쐬
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
}

void Scene::onKeyUp(UINT8 key)
{
	if (key == 'Z') {
		this->mMode = static_cast<decltype(this->mMode)>((this->mMode + 1) % eMODE_COUNT);
		this->updateTitle();
	}
}

void Scene::onRender()
{
	//GPU�ɕK�v�ȃf�[�^��ݒ肷��
	switch (this->mMode) {
	case eMODE_INDEX:	this->mpImmediateContext->CSSetShader(this->mpCSClearScreen.Get(), nullptr, 0); break;
	case eMODE_SAMPLER:	this->mpImmediateContext->CSSetShader(this->mpCSClearScreenWithSampler.Get(), nullptr, 0); break;
	default:
		assert(false);
	}

	std::array<ID3D11ShaderResourceView*, 1> ppSRVs = { { this->mpTex2DSRV.Get(), } };
	this->mpImmediateContext->CSSetShaderResources(0, static_cast<UINT>(ppSRVs.size()), ppSRVs.data());

	std::array<ID3D11SamplerState*, 1> ppSamplers = { { this->mpSampler.Get(), } };
	this->mpImmediateContext->CSSetSamplers(0, static_cast<UINT>(ppSamplers.size()), ppSamplers.data());

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

void Scene::updateTitle()
{
	std::wstring suffix;
	switch (this->mMode) {
	case eMODE_INDEX: suffix = L"�Y�����A�N�Z�X"; break;
	case eMODE_SAMPLER: suffix = L"�T���v���A�N�Z�X"; break;
	}
	this->setCustomWindowText(suffix.c_str());
}