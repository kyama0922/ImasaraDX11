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
	this->updateTitle();

	auto instanceData = this->makeInstanceData();

	{//�O���t�B�b�N�X�p�C�v���C���̏�����
		std::vector<char> byteCode;
		createShader(this->mpVertexShader.GetAddressOf(), this->mpDevice.Get(), "VertexShader.cso", &byteCode);

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

		//�X�e���V���e�X�g�̂��߂̎O�p�`�o�b�t�@�p�̒��_�V�F�[�_
		createShader(this->mpVSLargeTriangle.GetAddressOf(), this->mpDevice.Get(), "VSLargeTriangle.cso", &byteCode);

		//�s�N�Z���V�F�[�_
		createShader(this->mpPSEarlyZ.GetAddressOf(), this->mpDevice.Get(), "PSEarlyZ.cso", &byteCode);
	}
	{//�C���X�^���X�`��p�̃o�b�t�@�쐬
		CreateStructuredBuffer(this->mpOffsetBuffer.GetAddressOf(), this->mpOffsetBufferSRV.GetAddressOf(), nullptr, this->mpDevice.Get(), M_INSTANCED_COUNT, instanceData.data());
	}
	{//IA�p�̃o�b�t�@�쐬
		std::array<Vertex, 6> data = { {
			{ {  0.0f,  0.5f, 0 } },
			{ {  0.5f, -0.5f, 0 } },
			{ { -0.5f, -0.5f, 0 } },
		} };
		CreateIABuffer(this->mpVertexBuffer.GetAddressOf(), this->mpDevice.Get(), static_cast<UINT>(data.size()), data.data(), D3D11_BIND_VERTEX_BUFFER);

		std::array<uint16_t, 3> indices = { {
			0, 1, 2,
		} };
		CreateIABuffer(this->mpIndexBuffer.GetAddressOf(), this->mpDevice.Get(), static_cast<UINT>(indices.size()), indices.data(), D3D11_BIND_INDEX_BUFFER);

		std::array<Vertex, 6> largeTriangle = { {
			{ { 0.0f,  0.5f, 0 } },
			{ { 0.5f, -0.5f, 0 } },
			{ { -0.5f, -0.5f, 0 } },
		} };
		CreateIABuffer(this->mpLargeTriangleBuffer.GetAddressOf(), this->mpDevice.Get(), static_cast<UINT>(largeTriangle.size()), largeTriangle.data(), D3D11_BIND_VERTEX_BUFFER);
	}
	{//�[�x�X�e���V���o�b�t�@�̍쐬
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = this->width();
		desc.Height = this->height();
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.SampleDesc.Count = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		auto hr = this->mpDevice->CreateTexture2D(&desc, nullptr, this->mpDepthStencil.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�[�x�X�e���V���o�b�t�@�̍쐬�Ɏ��s");
		}
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = desc.Format;
		dsvDesc.Texture2D.MipSlice = 0;
		hr = this->mpDevice->CreateDepthStencilView(this->mpDepthStencil.Get(), &dsvDesc, this->mpDepthStencilDSV.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�[�x�X�e���V���r���[�쐬�Ɏ��s");
		}
	}
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = true;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.StencilEnable = false;
		auto hr = this->mpDevice->CreateDepthStencilState(&desc, this->mpDSDepthTest.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�[�x�e�X�g�p�̃X�e�[�g�쐬�Ɏ��s");
		}

		//�X�e���V���e�X�g�p�̃X�e�[�g�쐬
		desc.DepthEnable = false;
		desc.StencilEnable = true;
		desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;

		desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
		desc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;

		hr = this->mpDevice->CreateDepthStencilState(&desc, this->mpDSStencilTest.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�X�e���V���e�X�g�p�̃X�e�[�g�쐬�Ɏ��s");
		}
	}
	{//�u�����h�X�e�[�g�̍쐬
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		auto hr = this->mpDevice->CreateBlendState(&desc, this->mpBlendState.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�u�����h�X�e�[�g�̍쐬�Ɏ��s");
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

	//���̓A�Z���u���X�e�[�W
	std::array<ID3D11Buffer*, 1> ppBufs = { {
			this->mpVertexBuffer.Get(),
	} };
	std::array<UINT, 1> strides = { { sizeof(Vertex) } };
	std::array<UINT, 1> offsets = { { 0 } };
	this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppBufs.size()), ppBufs.data(), strides.data(), offsets.data());
	this->mpImmediateContext->IASetIndexBuffer(this->mpIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	this->mpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->mpImmediateContext->IASetInputLayout(this->mpInputLayout.Get());

	//���_�V�F�[�_
	this->mpImmediateContext->VSSetShader(this->mpVertexShader.Get(), nullptr, 0);
	std::array<ID3D11ShaderResourceView*, 1> ppSRVs = { {
			this->mpOffsetBufferSRV.Get(),
		} };
	this->mpImmediateContext->VSSetShaderResources(0, static_cast<UINT>(ppSRVs.size()), ppSRVs.data());

	//�s�N�Z���V�F�[�_
	this->mpImmediateContext->PSSetShader(this->mpPixelShader.Get(), nullptr, 0);

	//�A�E�g�v�b�g�}�[�W���X�e�[�W
	std::array<ID3D11RenderTargetView*, 1> ppRTVs = { {
			this->mpBackBufferRTV.Get(),
		} };
	this->mpImmediateContext->ClearDepthStencilView(this->mpDepthStencilDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	switch (this->mMode) {
	case eMODE_NONE:
		this->mpImmediateContext->OMSetRenderTargets(static_cast<UINT>(ppRTVs.size()), ppRTVs.data(), nullptr);
		this->mpImmediateContext->OMSetDepthStencilState(nullptr, 0);

		//���s
		this->mpImmediateContext->DrawIndexedInstanced(3, M_INSTANCED_COUNT, 0, 0, 0);
		break;
	case eMODE_DEPTH_TEST:
		this->mpImmediateContext->OMSetRenderTargets(static_cast<UINT>(ppRTVs.size()), ppRTVs.data(), this->mpDepthStencilDSV.Get());
		this->mpImmediateContext->OMSetDepthStencilState(this->mpDSDepthTest.Get(), 0);

		//���s
		this->mpImmediateContext->DrawIndexedInstanced(3, M_INSTANCED_COUNT, 0, 0, 0);
		break;
	case eMODE_STENCIL_TEST:
		this->mpImmediateContext->OMSetRenderTargets(static_cast<UINT>(ppRTVs.size()), ppRTVs.data(), this->mpDepthStencilDSV.Get());
		this->mpImmediateContext->OMSetDepthStencilState(this->mpDSStencilTest.Get(), 0);

		//�^�����p�ɑ傫�ȎO�p�`��`��
		ppBufs[0] = this->mpLargeTriangleBuffer.Get();
		this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppBufs.size()), ppBufs.data(), strides.data(), offsets.data());
		this->mpImmediateContext->VSSetShader(this->mpVSLargeTriangle.Get(), nullptr, 0);
		this->mpImmediateContext->PSSetShader(nullptr, nullptr, 0);
		this->mpImmediateContext->Draw(3, 0);

		//�X�e���V���e�X�g�ɂ��A��̎O�p�`���`�悳��Ă��Ȃ������������`�悳���
		ppBufs[0] = this->mpVertexBuffer.Get();
		this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppBufs.size()), ppBufs.data(), strides.data(), offsets.data());
		this->mpImmediateContext->VSSetShader(this->mpVertexShader.Get(), nullptr, 0);
		this->mpImmediateContext->PSSetShader(this->mpPixelShader.Get(), nullptr, 0);

		this->mpImmediateContext->DrawIndexedInstanced(3, M_INSTANCED_COUNT, 0, 0, 0);
		break;
	case eMODE_EARLY_Z:
	{
		//���Z�o�b�t�@�ɏ�������
		this->mpImmediateContext->PSSetShader(nullptr, nullptr, 0);
		this->mpImmediateContext->OMSetDepthStencilState(this->mpDSDepthTest.Get(), 0);

		this->mpImmediateContext->DrawIndexedInstanced(3, M_INSTANCED_COUNT, 0, 0, 0);

		//EarlyZ
		//���Z�������s���u�����h�X�e�[�g���w�肵�Ă���̂ŁA�O�p�`�̂ǂ̕��������Z����Ă��邩���ڂ��Ă�������
		std::array<float, 4> factor = { { 1, 1, 1, 1 } };
		this->mpImmediateContext->OMSetBlendState(this->mpBlendState.Get(), factor.data(), 0xffffffff);
		this->mpImmediateContext->PSSetShader(this->mpPSEarlyZ.Get(), nullptr, 0);
		this->mpImmediateContext->DrawIndexedInstanced(3, M_INSTANCED_COUNT, 0, 0, 0);

		this->mpImmediateContext->OMSetBlendState(nullptr, factor.data(), 0xffffffff);
		break;
	}
	default:
		assert(false);
	}

}

void Scene::onDestroy()
{
}

void Scene::updateTitle()
{
	std::wstring title = L"";
	switch (this->mMode) {
	case eMODE_NONE:			title += L"eMODE_NONE"; break;
	case eMODE_DEPTH_TEST:		title += L"eMODE_DEPTH_TEST"; break;
	case eMODE_STENCIL_TEST:	title += L"eMODE_STENCIL_TEST"; break;
	case eMODE_EARLY_Z:			title += L"eMODE_EARLY_Z"; break;
	default:
		assert(false && "������");
	}

	this->setCustomWindowText(title.c_str());
}

std::vector<Scene::InstancedParam> Scene::makeInstanceData()const
{
	int i = 0;
	float work = 0.f;
	auto calPos = [&i, &work, this]() {
		const float start = -0.5f;
		const float interval = 1.f / this->M_INSTANCED_COUNT;
		auto r =  start + work;
		work += interval;
		return DirectX::SimpleMath::Vector3(r, 0, ((i++ % 2) == 0) ? 0 : 0.5f);
	};

	std::vector<InstancedParam> data;
	data.resize(this->M_INSTANCED_COUNT);
	data.at(0).offset = calPos();
	data.at(0).color = DirectX::SimpleMath::Vector4(1, 0, 0, 1);

	data.at(1).offset = calPos();
	data.at(1).color = DirectX::SimpleMath::Vector4(0, 1, 0, 1);

	data.at(2).offset = calPos();
	data.at(2).color = DirectX::SimpleMath::Vector4(0, 0, 1, 1);

	data.at(3).offset = calPos();
	data.at(3).color = DirectX::SimpleMath::Vector4(1, 1, 0, 1);

	data.at(4).offset = calPos();
	data.at(4).color = DirectX::SimpleMath::Vector4(0, 1, 1, 1);

	return data;
}