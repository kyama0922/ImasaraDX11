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
#include <sstream>

Scene::Scene(UINT width, UINT height, std::wstring name)
	: DXSample(width, height, name)
{
}

void Scene::onInit()
{
	this->createScreenTexture();
	this->updateTitle();

	struct SphereInfo {
		DirectX::SimpleMath::Vector3 pos;
		float range;
		DirectX::SimpleMath::Vector3 color;
		float pad;//�\���̂̃T�C�Y��16�̔{���ɂ��邽�߂ɐ錾���Ă���
	};

	{//�萔�o�b�t�@���g�������̕`��̏�����
		createShader(this->mpCSRenderSphereByCB.GetAddressOf(), this->mpDevice.Get(), "RenderSphereByConstantBuffer.cso");

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

	{//StructuredBuffer���g�������̕`��̏�����
		createShader(this->mpCSRenderSphereByStructured.GetAddressOf(), this->mpDevice.Get(), "RenderSphereByStructuredBuffer.cso");

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(SphereInfo);

		desc.ByteWidth = sizeof(SphereInfo);
		desc.Usage = D3D11_USAGE_DEFAULT;

		SphereInfo info;
		info.pos = DirectX::SimpleMath::Vector3(10, 0, 30.f);
		info.range = 5.f;
		info.color = DirectX::SimpleMath::Vector3(0.4f, 1.f, 0.4f);
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &info;
		initData.SysMemPitch = sizeof(info);
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpStructuredBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�\�����o�b�t�@�̍쐬�Ɏ��s");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 1;
		// ���̃R�[�h�ł��ݒ�o����
		//srvDesc.Buffer.ElementOffset = 0;
		//srvDesc.Buffer.ElementWidth = 1;
		// ViewDimension��D3D11_SRV_DIMENSION_BUFFEREX�Ɏw���OK
		//srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		//srvDesc.BufferEx.FirstElement = 0;
		//srvDesc.BufferEx.Flags = 0;
		//srvDesc.BufferEx.NumElements = 1;

		hr = this->mpDevice->CreateShaderResourceView(this->mpStructuredBuffer.Get(), &srvDesc, this->mpStructuredBufferSRV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�\�����o�b�t�@��ShaderResourceView�̍쐬�Ɏ��s");
		}
	}

	{//�o�C�g�A�h���X�o�b�t�@���g�������̕`��̏�����
		createShader(this->mpCSRenderSphereByByteAddress.GetAddressOf(), this->mpDevice.Get(), "RenderSphereByByteAddressBuffer.cso");

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

		desc.ByteWidth = sizeof(SphereInfo);
		desc.Usage = D3D11_USAGE_DEFAULT;

		SphereInfo info;
		info.pos = DirectX::SimpleMath::Vector3(-15, 0, 45.f);
		info.range = 15.f;
		info.color = DirectX::SimpleMath::Vector3(0.4f, 0.4f, 1.f);
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &info;
		initData.SysMemPitch = sizeof(info);
		auto hr = this->mpDevice->CreateBuffer(&desc, &initData, this->mpByteAddressBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�o�C�g�A�h���X�o�b�t�@�̍쐬�Ɏ��s");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.BufferEx.FirstElement = 0;
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.BufferEx.NumElements = sizeof(SphereInfo) / 4;
		hr = this->mpDevice->CreateShaderResourceView(this->mpByteAddressBuffer.Get(), &srvDesc, this->mpByteAddressBufferSRV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�o�C�g�A�h���X�o�b�t�@��ShaderResourceView�̍쐬�Ɏ��s");
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

	this->runStackBuffer();
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
	case eMODE_CONSTANT_BUFFER:
	{
		this->mpImmediateContext->CSSetShader(this->mpCSRenderSphereByCB.Get(), nullptr, 0);
		std::array<ID3D11Buffer*, 1> ppCBs = { {
				this->mpConstantBuffer.Get(),
			} };
		this->mpImmediateContext->CSSetConstantBuffers(1, static_cast<UINT>(ppCBs.size()), ppCBs.data());
	}
		break;
	case eMODE_STRUCTURED_BUFFER:
	{
		this->mpImmediateContext->CSSetShader(this->mpCSRenderSphereByStructured.Get(), nullptr, 0);

		std::array < ID3D11ShaderResourceView*, 1> ppSRVs = { {
				this->mpStructuredBufferSRV.Get(),
			} };
		this->mpImmediateContext->CSSetShaderResources(0, static_cast<UINT>(ppSRVs.size()), ppSRVs.data());
		break;
	}
	case eMODE_BYTE_ADDRESS_BUFFER:
	{
		this->mpImmediateContext->CSSetShader(this->mpCSRenderSphereByByteAddress.Get(), nullptr, 0);

		std::array < ID3D11ShaderResourceView*, 1> ppSRVs = { {
				this->mpByteAddressBufferSRV.Get(),
			} };
		this->mpImmediateContext->CSSetShaderResources(0, static_cast<UINT>(ppSRVs.size()), ppSRVs.data());
		break;
	}
	default:
		assert(false);
	}

	std::array<ID3D11Buffer*, 1> ppCBs = { {
			this->mpCBCamera.Get(),
		} };
	this->mpImmediateContext->CSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());

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

void Scene::runStackBuffer()
{
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pCSPushStack;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pCSPopStack;
	//�X�^�b�N������s���o�b�t�@�ƃr���[
	Microsoft::WRL::ComPtr<ID3D11Buffer> pStackBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pStackBufferUAV;
	//�X�^�b�N������s���o�b�t�@����f�[�^���󂯎��o�b�t�@�ƃr���[
	Microsoft::WRL::ComPtr<ID3D11Buffer> pRWBuffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pRWBufferUAV;
	//GPU����CPU�Ƀf�[�^��]�����邽�߂Ɏg���o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> pReadBuffer;

	createShader(pCSPushStack.GetAddressOf(), this->mpDevice.Get(), "PushStack.cso");
	createShader(pCSPopStack.GetAddressOf(), this->mpDevice.Get(), "PopStack.cso");

	struct Data {
		DirectX::SimpleMath::Vector4 color;
	};
	const UINT count = 10;
	{//�X�^�b�N������s���o�b�t�@�쐬
		//AppendStructuredBuffer��ConsumeStructuredBuffer��StructuredBuffer�Ɠ����ݒ�Ńo�b�t�@���쐬����
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.ByteWidth = sizeof(Data) * count;//�X�^�b�N�̏�������߂Ă���
		desc.StructureByteStride = sizeof(Data);
		desc.Usage = D3D11_USAGE_DEFAULT;
		auto hr = this->mpDevice->CreateBuffer(&desc, nullptr, pStackBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�X�^�b�N����p�̃o�b�t�@�쐬�Ɏ��s");
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = count;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
		hr = this->mpDevice->CreateUnorderedAccessView(pStackBuffer.Get(), &uavDesc, pStackBufferUAV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�X�^�b�N����p�̃o�b�t�@��UAV�̍쐬�Ɏ��s");
		}
	}
	{//�f�[�^���󂯎��o�b�t�@�쐬
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.ByteWidth = sizeof(Data) * count;
		desc.StructureByteStride = sizeof(Data);
		desc.Usage = D3D11_USAGE_DEFAULT;
		auto hr = this->mpDevice->CreateBuffer(&desc, nullptr, pRWBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�f�[�^�󂯎��p�̃o�b�t�@�쐬�Ɏ��s");
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = count;
		hr = this->mpDevice->CreateUnorderedAccessView(pRWBuffer.Get(), &uavDesc, pRWBufferUAV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�f�[�^�󂯎��p�̃o�b�t�@��UAV�̍쐬�Ɏ��s");
		}
	}
	{//GPU����CPU�Ƀf�[�^��]�����邽�߂̃o�b�t�@�쐬
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Data) * count;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		auto hr = this->mpDevice->CreateBuffer(&desc, nullptr, pReadBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("GPU����CPU�փf�[�^��]�����邽�߂̃o�b�t�@�쐬�Ɏ��s");
		}
	}

	//------------------------------------------------------------
	//	�V�F�[�_���s
	std::array<ID3D11UnorderedAccessView*, 2> ppUAVs = { {
		pStackBufferUAV.Get(),
		pRWBufferUAV.Get(),
	} };
	std::array<UINT, 2> pInitCounter = { {
			0, 0
		} };

	//�v�b�V��������s��
	this->mpImmediateContext->CSSetShader(pCSPushStack.Get(), nullptr, 0);
	pInitCounter[0] = 0;//�X�^�b�N�̌���0�ɂ��Ă���
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, 1, ppUAVs.data(), pInitCounter.data());
	this->mpImmediateContext->Dispatch(count, 1, 1);

	//�|�b�v������s��
	this->mpImmediateContext->CSSetShader(pCSPopStack.Get(), nullptr, 0);
	pInitCounter[0] = -1;//-1���w�肷��ƌ��݂̃f�[�^�̌�������J�E���^�ɐݒ肵�Ă����
	pInitCounter[1] = -1;
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), pInitCounter.data());
	this->mpImmediateContext->Dispatch(count, 1, 1);

	//CPU���Ƀf�[�^��]�����邽�߂�GPU����o�b�t�@�̐ݒ���O���B
	ppUAVs[0] = ppUAVs[1] = nullptr;
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), pInitCounter.data());

	this->mpImmediateContext->CopyResource(pReadBuffer.Get(), pRWBuffer.Get());

	D3D11_MAPPED_SUBRESOURCE mapped;
	auto hr = this->mpImmediateContext->Map(pReadBuffer.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	if (SUCCEEDED(hr)) {
		//CPU�Ƀf�[�^��]���ł������Ƃ��m�F���邽�߂ɑS���̃f�[�^���o�͂��Ă���B
		auto* pData = static_cast<DirectX::SimpleMath::Vector4*>(mapped.pData);
		OutputDebugStringA("�X�^�b�N������s���o�b�t�@�̃e�X�g�R�[�h\n");
		for (UINT i = 0; i < count; ++i) {
			const auto& v = pData[i];
			std::stringstream str;
			str << "index=" << i << ":";
			str << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
			str << std::endl;
			OutputDebugStringA(str.str().c_str());
		}
		this->mpImmediateContext->Unmap(pReadBuffer.Get(), 0);
		OutputDebugStringA("---------------------------------------\n");
	}
}


void Scene::updateTitle()
{
	std::wstring title;
	switch (this->mMode) {
	case eMODE_CONSTANT_BUFFER:		title = L"�萔�o�b�t�@"; break;
	case eMODE_STRUCTURED_BUFFER:	title = L"StructuredBuffer"; break;
	case eMODE_BYTE_ADDRESS_BUFFER: title = L"ByteAddressBuffer"; break;
	default:
		assert(false && "������");
	}
	this->setCustomWindowText(title.c_str());
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
