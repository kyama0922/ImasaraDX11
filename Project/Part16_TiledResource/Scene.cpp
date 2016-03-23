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

Scene::Scene(UINT width, UINT height, std::wstring name)
	: DXSample(width, height, name)
{
}

void Scene::onInit()
{
	//�^�C�����\�[�X�ɑΉ����Ă��邩�`�F�b�N
	D3D11_FEATURE_DATA_D3D11_OPTIONS1 featureData;
	this->mpDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &featureData, sizeof(featureData));
	if (D3D11_TILED_RESOURCES_NOT_SUPPORTED == featureData.TiledResourcesTier) {
		throw std::runtime_error("�g�p���Ă���f�o�C�X�̓^�C�����\�[�X�ɑΉ����Ă��܂���B");
	}

	auto hr = this->mpImmediateContext.Get()->QueryInterface(IID_PPV_ARGS(&mpContext2));
	if (FAILED(hr)) {
		throw std::runtime_error("DeviceContext2���g���܂���");
	}

	{//�^�C�����\�[�X�̍쐬
		auto desc = makeTex2DDesc(this->width(), this->height(), DXGI_FORMAT_R8G8B8A8_UNORM);
		desc.MiscFlags = D3D11_RESOURCE_MISC_TILED;
		desc.Width = this->width();
		desc.Height = this->height();
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		//desc.MipLevels = calMaxMipLevel(desc.Width, desc.Height);
		auto create = [&](TileTexture* pOut) {
			auto hr = this->mpDevice->CreateTexture2D(&desc, nullptr, pOut->mpResource.GetAddressOf());
			if (FAILED(hr)) {
				throw std::runtime_error("�^�C�����\�[�X�̍쐬�Ɏ��s");
			}

			hr = this->mpDevice->CreateShaderResourceView(pOut->mpResource.Get(), nullptr, pOut->mpSRV.GetAddressOf());
			if (FAILED(hr)) {
				throw std::runtime_error("�^�C�����\�[�X�̃V�F�[�_���\�[�X�r���[�̍쐬�Ɏ��s");
			}
			hr = this->mpDevice->CreateUnorderedAccessView(pOut->mpResource.Get(), nullptr, pOut->mpUAV.GetAddressOf());
			if (FAILED(hr)) {
				throw std::runtime_error("�^�C�����\�[�X�̃A���I�[�_�[�h�A�N�Z�X�r���[�̍쐬�Ɏ��s");
			}
		};

		this->mTextures.resize(6);
		for (auto& tex : this->mTextures) {
			create(&tex);
		}
		create(&this->mTargetTex);
	}

	{//�^�C���v�[���̍쐬
		Microsoft::WRL::ComPtr<ID3D11Device2> pDevice2;
		auto hr = this->mpDevice.Get()->QueryInterface(IID_PPV_ARGS(&pDevice2));
		if (FAILED(hr)) {
			throw std::runtime_error("Device2���g���܂���B");
		}

		//�^�C�����\�[�X�̏����擾����
		TileInfo tileInfo;
		tileInfo.subresourceTileCount = 1;
		tileInfo.firstSubresourceTile = 0;
		pDevice2->GetResourceTiling(this->mTargetTex.mpResource.Get(), &tileInfo.tileCount, &tileInfo.packedMipDesc, &tileInfo.tileShape, &tileInfo.subresourceTileCount, tileInfo.firstSubresourceTile, &tileInfo.subresourceTiling);
		for (auto& tex : this->mTextures) {
			tex.mInfo = tileInfo;
		}
		this->mTargetTex.mInfo = tileInfo;

		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = 64 * 1024;//64KB�̔{���łȂ��Ƃ����Ȃ�
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_TILE_POOL;
		hr = this->mpDevice->CreateBuffer(&desc, nullptr, this->mpTilePool.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�^�C���v�[���̍쐬�Ɏ��s");
		}

		UINT64 unitSize = tileInfo.tileCount * (64 * 1024);
		hr = this->mpContext2->ResizeTilePool(this->mpTilePool.Get(), unitSize * this->mTextures.size());
		if (FAILED(hr)) {
			throw std::runtime_error("�^�C���v�[���̃��T�C�Y�Ɏ��s");
		}
	}
	{//�^�C�����\�[�X�Ƀ^�C���v�[����ݒ肷��
		D3D11_TEXTURE2D_DESC desc;
		this->mTargetTex.mpResource->GetDesc(&desc);
		auto& tileInfo = this->mTargetTex.mInfo;

		std::array<D3D11_TILE_REGION_SIZE, 1 > regionSizes;
		regionSizes[0].Width = tileInfo.subresourceTiling.WidthInTiles;
		regionSizes[0].Height = tileInfo.subresourceTiling.HeightInTiles;
		regionSizes[0].Depth = tileInfo.subresourceTiling.DepthInTiles;
		regionSizes[0].bUseBox = true;
		regionSizes[0].NumTiles = regionSizes[0].Width * regionSizes[0].Height * regionSizes[0].Depth;

		for (UINT i = 0; i < this->mTextures.size(); ++i) {
			auto& tex = this->mTextures[i];

			//�^�C�����\�[�X�P�ʂ̍��W�l
			std::array<D3D11_TILED_RESOURCE_COORDINATE, 1> coordinates;
			coordinates[0].Subresource = 0;
			coordinates[0].X = 0;
			coordinates[0].Y = 0;
			coordinates[0].Z = 0;

			std::array<UINT, 1> ranges = { { 0 } };
			std::array<UINT, 1> offsets = { { i * tileInfo.tileCount } };
			std::array<UINT, 1> rangeTileCounts = { { tileInfo.tileCount } };
			UINT flags = 0;


			hr = this->mpContext2->UpdateTileMappings(
				tex.mpResource.Get(), static_cast<UINT>(coordinates.size()), coordinates.data(), regionSizes.data(),
				this->mpTilePool.Get(), static_cast<UINT>(ranges.size()), /*ranges.data()*/nullptr, offsets.data(), rangeTileCounts.data(), flags);
			if (FAILED(hr)) {
				throw std::runtime_error("�^�C�����\�[�X�������^�C���v�[���̏ꏊ�̐ݒ�Ɏ��s");
			}
		}

		//�^�C���v�[���̓����ꏊ���}�b�v���邱�Ƃ��ł���
		std::array<D3D11_TILED_RESOURCE_COORDINATE, 1> coordinates;
		coordinates[0].Subresource = 0;
		coordinates[0].X = 0;
		coordinates[0].Y = 0;
		coordinates[0].Z = 0;

		std::array<UINT, 1> ranges = { { 0 } };
		std::array<UINT, 1> offsets = { { 0 } };
		std::array<UINT, 1> rangeTileCounts = { { tileInfo.tileCount } };
		UINT flags = 0;
		hr = this->mpContext2->UpdateTileMappings(
			this->mTargetTex.mpResource.Get(), static_cast<UINT>(coordinates.size()), coordinates.data(), regionSizes.data(),
			this->mpTilePool.Get(), static_cast<UINT>(ranges.size()), /*ranges.data()*/nullptr, offsets.data(), rangeTileCounts.data(), flags);
		if (FAILED(hr)) {
			throw std::runtime_error("�^�C�����\�[�X�������^�C���v�[���̏ꏊ�̐ݒ�Ɏ��s");
		}
	}

	{
		createShader(this->mpCSDrawTexture.GetAddressOf(), this->mpDevice.Get(), "CSDrawTexture.cso");

		DirectX::SimpleMath::Vector4 color(1,1, 0, 1);
		createConstantBuffer(this->mpCBParam.GetAddressOf(), this->mpDevice.Get(), &color);

		//this->mpContext2->Flush();
		std::array<DirectX::SimpleMath::Vector4, 5> colorTable = { {
			{ 1, 0, 0, 1 },
			{ 0, 1, 0, 1 },
			{ 0, 0, 1, 1 },
			{ 1, 1, 0, 1 },
			{ 1, 0, 1, 1 },
		} };
		for (UINT i = 0; i < this->mTextures.size()-1; ++i) {
			auto& tex = this->mTextures[i];

			this->mpImmediateContext->UpdateSubresource(this->mpCBParam.Get(), 0, nullptr, &colorTable[i], sizeof(colorTable[i]), sizeof(colorTable[i]));

			//���߂Ďg���̂�GPU�ɓ`����K�v�͂Ȃ�
			//this->mpContext2->TiledResourceBarrier(tex.mpResource.Get(), tex.mpUAV.Get());

			this->mpImmediateContext->CSSetShader(this->mpCSDrawTexture.Get(), nullptr, 0);
			std::array<ID3D11Buffer*, 1> ppCBs = { { this->mpCBParam.Get() } };
			this->mpImmediateContext->CSSetConstantBuffers(0, static_cast<UINT>(ppCBs.size()), ppCBs.data());
			std::array<ID3D11UnorderedAccessView*, 1> ppUAVs = { { tex.mpUAV.Get() } };
			std::array<UINT, 1> pInitValues = { { 0 } };
			this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), pInitValues.data());

			auto dx = calDispatchCount(this->width(), 8);
			auto dy = calDispatchCount(this->height(), 8);
			this->mpImmediateContext->Dispatch(dx, dy, 1);

			ppUAVs[0] = nullptr;
			this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), pInitValues.data());

			//�R�s�[���Ƃ��Ĉ����Ă����悤GPU�ɓ`����
			//���ꂪ�Ȃ���΁AEXECUTION WARNING #3146139: NEED_TO_CALL_TILEDRESOURCEBARRIER�ƌx�����o��
			this->mpContext2->TiledResourceBarrier(tex.mpUAV.Get(), tex.mpResource.Get());
		}

		{//UpdateSubresource�̂悤��CPU����f�[�^�𑗂邱�Ƃ��ł���
			auto& last = *this->mTextures.rbegin();
			auto& tileInfo = last.mInfo;
			std::vector<uint32_t> src;
			src.resize(tileInfo.tileCount * (64 * 1024 / sizeof(uint32_t)));//�K��64KB�̔{���̃f�[�^���ɂȂ�悤�ɂ���

			D3D11_TEXTURE2D_DESC desc;
			this->mTargetTex.mpResource->GetDesc(&desc);
			UINT size = sqrt(64 * 1024 / sizeof(uint32_t));
			for (UINT tileIndex = 0; tileIndex < tileInfo.tileCount; ++tileIndex) {
				//�^�C���P�ʂɃf�[�^��ݒ肷��K�v������
				UINT offset = tileIndex * (128 * 128);
				for (UINT y = 0; y < size; ++y) {
					for (UINT x = 0; x < size; ++x) {
						UINT index = y*size + x + offset;
						uint32_t color;
						if (((x + y) / 60) % 2 == 0) {
							color = 0xffffff00;
						} else {
							color = 0xff000000;
						}
						src[index] = color;
					}
				}
			}
			std::array<D3D11_TILED_RESOURCE_COORDINATE, 1> coordinates;
			coordinates[0].Subresource = 0;
			coordinates[0].X = 0;
			coordinates[0].Y = 0;
			coordinates[0].Z = 0;
			std::array<D3D11_TILE_REGION_SIZE, 1 > regionSizes;
			regionSizes[0].Width = tileInfo.subresourceTiling.WidthInTiles;
			regionSizes[0].Height = tileInfo.subresourceTiling.HeightInTiles;
			regionSizes[0].Depth = tileInfo.subresourceTiling.DepthInTiles;
			regionSizes[0].bUseBox = true;
			regionSizes[0].NumTiles = regionSizes[0].Width * regionSizes[0].Height * regionSizes[0].Depth;
			this->mpContext2->UpdateTiles(last.mpResource.Get(), coordinates.data(), regionSizes.data(), src.data(), 0);
			//CPU����f�[�^�𑗂�I�������Ƃ�GPU�ɓ`����
			this->mpContext2->TiledResourceBarrier(last.mpResource.Get(), last.mpResource.Get());
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
}

void Scene::onKeyUp(UINT8 key)
{
	if (key == 'Z') {
		this->mMode = static_cast<decltype(this->mMode)>((this->mMode + 1) % eMODE_COUNT);
	}
	bool isUpdateMapping = false;
	if (key == VK_LEFT && 0 < this->mTargetIndex) {
		--this->mTargetIndex;
		isUpdateMapping = true;
	}
	if (key == VK_RIGHT && this->mTargetIndex < this->mTextures.size()-1) {
		++this->mTargetIndex;
		isUpdateMapping = true;
	}

	if (isUpdateMapping) {
		auto& tileInfo = this->mTargetTex.mInfo;
		//�}�b�v�����R�s�[����
		std::array<D3D11_TILED_RESOURCE_COORDINATE, 1> coordinates;
		coordinates[0].Subresource = 0;
		coordinates[0].X = 0;
		coordinates[0].Y = 0;
		coordinates[0].Z = 0;
		std::array<D3D11_TILE_REGION_SIZE, 1 > regionSizes;
		regionSizes[0].Width = tileInfo.subresourceTiling.WidthInTiles;
		regionSizes[0].Height = tileInfo.subresourceTiling.HeightInTiles;
		regionSizes[0].Depth = tileInfo.subresourceTiling.DepthInTiles;
		regionSizes[0].bUseBox = true;
		regionSizes[0].NumTiles = regionSizes[0].Width * regionSizes[0].Height * regionSizes[0].Depth;

		std::array<UINT, 1> ranges = { { 0 } };
		std::array<UINT, 1> offsets = { { 0 } };
		std::array<UINT, 1> rangeTileCounts = { { tileInfo.tileCount } };
		UINT flags = 0;
		auto hr = this->mpContext2->CopyTileMappings(
			this->mTargetTex.mpResource.Get(), coordinates.data(),
			this->mTextures[this->mTargetIndex].mpResource.Get(), coordinates.data(), regionSizes.data(), flags);
		if (FAILED(hr)) {
			throw std::runtime_error("�^�C�����\�[�X�̃}�b�s���O���̃R�s�[�Ɏ��s");
		}
	}
}

void Scene::onRender()
{
	//GPU�ɕK�v�ȃf�[�^��ݒ肷��

	//�V�F�[�_�̌��ʂ��o�b�N�o�b�t�@�[�ɃR�s�[����
	this->mpImmediateContext->CopySubresourceRegion(this->mpBackBuffer.Get(), 0, 0, 0, 0, this->mTargetTex.mpResource.Get(), 0, nullptr);
}

void Scene::onDestroy()
{
}

