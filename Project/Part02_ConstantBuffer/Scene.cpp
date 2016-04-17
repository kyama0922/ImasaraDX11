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
	this->updateTitle();

	{//�萔�o�b�t�@�̍쐬
		Param param;	//ID3D11Buffer�ɐݒ肷��f�[�^
		param.clearColor = DirectX::SimpleMath::Vector4(1, 0.7f, 0.7f, 1.f);
		param.screenSize = DirectX::SimpleMath::Vector2(static_cast<float>(this->width()), static_cast<float>(this->height()));

		//�쐬����ID3D11Buffer�̐ݒ���
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(param) + (sizeof(param) % 16 == 0 ? 0 : 16 - sizeof(param) % 16);	//�T�C�Y��16�̔{���łȂ��Ƃ����Ȃ�
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//ID3D11Buffer��萔�o�b�t�@�Ƃ��Ďg���悤�錾���Ă���
		desc.Usage = D3D11_USAGE_DEFAULT;				//GPU�ォ�炵��ID3D11Buffer�̓��e�ɃA�N�Z�X�ł��Ȃ��悤�錾���Ă���
		desc.CPUAccessFlags = 0;						//CPU����̃A�N�Z�X�t���O�̎w��B����̓A�N�Z�X���Ȃ��̂ŉ����ݒ肵�Ȃ�

		//CPU��GPU�Ԃ̃f�[�^�]���̎��Ɏg���\����
		//�����ł�ID3D11Buffer�̏����f�[�^��ݒ肷�邽�߂Ɏg���Ă���
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &param;
		initData.SysMemPitch = sizeof(param);
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpCB.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�萔�o�b�t�@�̍쐬�Ɏ��s");
		}

	}
	{//�}�b�v�\�Ȓ萔�o�b�t�@�̍쐬
		Param param;	//ID3D11Buffer�ɐݒ肷��f�[�^
		param.clearColor = DirectX::SimpleMath::Vector4(0.7f, 1.f, 0.7f, 1.f);
		param.screenSize = DirectX::SimpleMath::Vector2(static_cast<float>(this->width()), static_cast<float>(this->height()));
		
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(param) + (sizeof(param) % 16 == 0 ? 0 : 16 - sizeof(param) % 16);//�T�C�Y��16�̔{���łȂ��Ƃ����Ȃ�
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	//ID3D11Buffer��萔�o�b�t�@�Ƃ��Ďg���悤�錾���Ă���
		desc.Usage = D3D11_USAGE_DYNAMIC;				//GPU��ł͓ǂݍ��݂�����CPU���珑�����݂������ł���悤�ɐ錾���Ă���
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//CPU��GPU�Ԃ̃f�[�^�]���̎��Ɏg���\����
		//�����ł�ID3D11Buffer�̏����f�[�^��ݒ肷�邽�߂Ɏg���Ă���
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = &param;
		initData.SysMemPitch = sizeof(param);

		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpCBMappable.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�}�b�v�\�Ȓ萔�o�b�t�@�̍쐬�Ɏ��s");
		}
	}

	{//��ʃN���A�[����V�F�[�_�̍쐬
		createShader(this->mpCSClearScreen.GetAddressOf(), this->mpDevice.Get(), "ClearScreen.cso");
	}

	{//�V�F�[�_�̏o�͐�̍쐬
		D3D11_TEXTURE2D_DESC desc = {};
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
	static float t = 0.0f;
	t += 0.01f;

	{//ID3D11DeviceCOntext::UpdateSubresource�֐����g�����萔�o�b�t�@�̍X�V
		Param param;
		param.clearColor = DirectX::SimpleMath::Vector4(abs(sin(t)), (cos(t)), (sin(t)), 1.f);
		param.screenSize.x = static_cast<float>(this->width());
		param.screenSize.y = static_cast<float>(this->height());

		this->mpImmediateContext->UpdateSubresource(this->mpCB.Get(), 0, nullptr, &param, 0, 0);
	}

	{//ID3D11DeviceContext::Map�֐����g�����萔�o�b�t�@�̍X�V
		UINT subresourceIndex = 0;
		D3D11_MAPPED_SUBRESOURCE mapped;
		auto hr = this->mpImmediateContext->Map(this->mpCBMappable.Get(), subresourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		if (SUCCEEDED(hr)) {
			Param* p = static_cast<Param*>(mapped.pData);
			p->clearColor = DirectX::SimpleMath::Vector4(abs(sin(t*2.f)), (cos(t)), (sin(t)), 1.f);
			p->screenSize.x = static_cast<float>(this->width());
			p->screenSize.y = static_cast<float>(this->height());
			//this->mpImmediateContext->Unmap(this->mpCBMappable.Get(), subresourceIndex);
		}
	}
}

void Scene::onKeyUp(UINT8 key)
{
}


void Scene::onRender()
{
	//���s����V�F�[�_��GPU�ɐݒ肷��
	this->mpImmediateContext->CSSetShader(this->mpCSClearScreen.Get(), nullptr, 0);

	std::array<ID3D11Buffer*, 1> ppCBs = { {
		this->mpCBMappable.Get(),
	} };
	this->mpImmediateContext->CSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());

	//�V�F�[�_��screen�Ƃ��Ĉ������\�[�X��ݒ肷��B
	std::array<ID3D11UnorderedAccessView*, 1> ppUAVs = { { this->mpScreenUAV.Get(), } };
	std::array<UINT, 1> initCounts = { { 0u, } };
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), initCounts.data());

	//ClearScreen.hlsl�̎��s
	this->mpImmediateContext->Dispatch(this->mWidth, this->mHeight, 1);

	//�V�F�[�_�̌��ʂ��o�b�N�o�b�t�@�ɃR�s�[����
	this->mpImmediateContext->CopySubresourceRegion(this->mpBackBuffer.Get(), 0, 0, 0, 0, this->mpScreen.Get(), 0, nullptr);
}

void Scene::onDestroy()
{
}

void Scene::updateTitle()
{
}
