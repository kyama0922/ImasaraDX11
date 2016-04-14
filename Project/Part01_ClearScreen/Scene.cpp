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

	{
		//���s���ɃV�F�[�_���R���p�C�����AID3D11ComputeShader���쐬����
		std::array<D3D_SHADER_MACRO, 2> macros = { {
			{"DEFINE_MACRO", "float4(0, 1, 1, 1)"},
			{nullptr, nullptr},
		} };
		UINT compileFlag = 0;
#ifdef _DEBUG
		compileFlag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		std::wstring filepath = L"Shader/ClearScreen.hlsl";
		const char* entryPoint = "main";
		//const char* entryPoint = "clearByOneThread"; // c++���ȉ�ʃN���A�V�F�[�_���g�������Ƃ��͂�������g�p���Ă�������
		const char* shaderTarget = "cs_5_0";
		Microsoft::WRL::ComPtr<ID3DBlob> pShaderBlob, pErrorMsg;
		//�V�F�[�_�̃R���p�C��
		HRESULT hr = D3DCompileFromFile(
			filepath.c_str(),
			macros.data(),
			nullptr,
			entryPoint,
			shaderTarget,
			compileFlag,
			0,
			pShaderBlob.GetAddressOf(),
			pErrorMsg.GetAddressOf());
		if (FAILED(hr)) {
			if (pErrorMsg) {
				OutputDebugStringA(static_cast<char*>(pErrorMsg->GetBufferPointer()));
			}
			throw std::runtime_error("ClearScreen.hlsl�̃R���p�C���Ɏ��s");
		}
		//ID3D11ComputeShader�̍쐬
		hr = this->mpDevice->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &this->mpCSClearScreen);
		if (FAILED(hr)) {
			throw std::runtime_error("ClearScreenWithConstantBuffer.hlsl�̍쐬�Ɏ��s");
		}
	}

	{//�R���p�C���ς݂̃V�F�[�_����ID3D11ComputeShader���쐬����R�[�h
		std::vector<char> byteCode;
		if (!loadBinaryFile(&byteCode, "ClearScreen.cso")) {
			throw std::runtime_error("�V�F�[�_�t�@�C���̓ǂݍ��݂Ɏ��s");
		}

		Microsoft::WRL::ComPtr<ID3D11ComputeShader> pShader;
		HRESULT hr;
		hr = this->mpDevice->CreateComputeShader(byteCode.data(), static_cast<SIZE_T>(byteCode.size()), nullptr, &pShader);
		if (FAILED(hr)) {
			throw std::runtime_error("ID3D11ComputerShader�̍쐬�Ɏ��s");
		}
	}

	{//�V�F�[�_�̏o�͐�̍쐬
		D3D11_TEXTURE2D_DESC desc = { };
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
}


void Scene::onRender()
{
	//���s����V�F�[�_��GPU�ɐݒ肷��
	this->mpImmediateContext->CSSetShader(this->mpCSClearScreen.Get(), nullptr, 0);

	//�V�F�[�_��screen�Ƃ��Ĉ������\�[�X��ݒ肷��B
	std::array<ID3D11UnorderedAccessView*, 1> ppUAVs = { { this->mpScreenUAV.Get(), } };
	std::array<UINT, 1> initCounts = { { 0u, } };
	this->mpImmediateContext->CSSetUnorderedAccessViews(0, static_cast<UINT>(ppUAVs.size()), ppUAVs.data(), initCounts.data());

	//ClearScreen.hlsl�̎��s
	this->mpImmediateContext->Dispatch(this->mWidth, this->mHeight, 1);

	//�⑫: �V�F�[�_���g�킸��ʂ��N���A����R�[�h
	//float value[4] = {1, 0.7f, 1, 1};//������ԁA�΁A�A�A���t�@
	//this->mpImmediateContext->ClearUnorderedAccessViewFloat(this->mpScreenUAV.Get(), value);

	//�V�F�[�_�̌��ʂ��o�b�N�o�b�t�@�[�ɃR�s�[����
	this->mpImmediateContext->CopySubresourceRegion(this->mpBackBuffer.Get(), 0, 0, 0, 0, this->mpScreen.Get(), 0, nullptr);
}

void Scene::onDestroy()
{
}

void Scene::updateTitle()
{
}
