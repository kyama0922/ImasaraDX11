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
		createShader(this->mpVertexShader.GetAddressOf(), this->mpDevice.Get(), "VertexShader.cso", &byteCode);

		//���̓��C�A�E�g�̍쐬
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> elements = { {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		} };
		auto hr = this->mpDevice->CreateInputLayout(elements.data(), static_cast<UINT>(elements.size()), byteCode.data(), byteCode.size(), this->mpInputLayout.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("���̓��C�A�E�g�̍쐬�Ɏ��s");
		}

		//�s�N�Z���V�F�[�_
		createShader(this->mpPixelShader.GetAddressOf(), this->mpDevice.Get(), "PixelShader.cso", &byteCode);
	}
	{//�X�g���[���A�E�g�v�b�g�p�V�F�[�_�̍쐬
		std::vector<char> byteCode;
		//���_�V�F�[�_
		createShader(this->mpVSStreamOutput.GetAddressOf(), this->mpDevice.Get(), "VSStreamOutput.cso", &byteCode);

		//�W�I���g���V�F�[�_
		if (!loadBinaryFile(&byteCode, "GeometryShader.cso")) {
			throw std::runtime_error("GeometryShader.cso�̓ǂݍ��݂Ɏ��s");
		}

		std::array<D3D11_SO_DECLARATION_ENTRY, 2> soEntrys = { {
			{ 0, "POSITION", 0, 0, 3, 0 },
			{ 0, "TEXCOORD", 0, 0, 4, 0 },
		} };
		std::array<UINT, 1> strides = { { sizeof(float) * 3 + sizeof(float) * 4 } };
		auto hr = this->mpDevice->CreateGeometryShaderWithStreamOutput(
			byteCode.data(),
			static_cast<SIZE_T>(byteCode.size()),
			soEntrys.data(),
			static_cast<UINT>(soEntrys.size()),
			strides.data(),
			static_cast<UINT>(strides.size()),
			D3D11_SO_NO_RASTERIZED_STREAM,
			nullptr,
			this->mpGeometryShader.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("GeometryShader�̍쐬�Ɏ��s");
		}

	}
	{//IA�p�̃o�b�t�@�쐬
		std::array<Vertex, 3> data = { {
			{ {  0.0f,  0.5f, 0 } },
			{ {  0.5f, -0.5f, 0 } },
			{ { -0.5f, -0.5f, 0 } },
		} };
		CreateIABuffer(this->mpVertexBuffer.GetAddressOf(), this->mpDevice.Get(), static_cast<UINT>(data.size()), data.data(), D3D11_BIND_VERTEX_BUFFER);
	}
	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Vertex) * M_STREAM_OUTPUT_COUNT;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
		auto hr = this->mpDevice->CreateBuffer(&desc, nullptr, this->mpStreamOutputBuffer.GetAddressOf());
		if (FAILED(hr)) {
			throw std::runtime_error("�X�g���[���A�E�g�v�b�g�p�̃o�b�t�@�쐬�Ɏ��s");
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
	//�O�p�`�𐶐�����
	this->mpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	this->mpImmediateContext->VSSetShader(this->mpVSStreamOutput.Get(), nullptr, 0);
	this->mpImmediateContext->GSSetShader(this->mpGeometryShader.Get(), nullptr, 0);
	std::array<ID3D11Buffer*, 1> ppSOBufs = { {
			this->mpStreamOutputBuffer.Get(),
		} };
	std::array<UINT, 1> soOffsets = { { 0 } };
	this->mpImmediateContext->SOSetTargets(static_cast<UINT>(ppSOBufs.size()), ppSOBufs.data(), soOffsets.data());

	this->mpImmediateContext->Draw(this->M_STREAM_OUTPUT_COUNT, 0);

	ppSOBufs[0] = nullptr;
	this->mpImmediateContext->SOSetTargets(static_cast<UINT>(ppSOBufs.size()), ppSOBufs.data(), soOffsets.data());
	ppSOBufs[0] = this->mpStreamOutputBuffer.Get();

	//���������O�p�`��`�悷��
	//���̓A�Z���u���X�e�[�W
	std::array<UINT, 1> strides = { { sizeof(Vertex) } };
	this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppSOBufs.size()), ppSOBufs.data(), strides.data(), soOffsets.data());
	this->mpImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->mpImmediateContext->IASetInputLayout(this->mpInputLayout.Get());

	//���_�V�F�[�_
	this->mpImmediateContext->VSSetShader(this->mpVertexShader.Get(), nullptr, 0);
	//�W�I���g���V�F�[�_
	this->mpImmediateContext->GSSetShader(nullptr, nullptr, 0);
	//�s�N�Z���V�F�[�_
	this->mpImmediateContext->PSSetShader(this->mpPixelShader.Get(), nullptr, 0);

	this->mpImmediateContext->DrawAuto();

	ppSOBufs[0] = nullptr;
	this->mpImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(ppSOBufs.size()), ppSOBufs.data(), strides.data(), soOffsets.data());

}

void Scene::onDestroy()
{
}
