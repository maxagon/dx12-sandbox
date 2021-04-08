#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <atlbase.h>
#include <DebugCheck.h>
#include <DescriptorHeapAllocator.h>

#include <memory>

struct ShaderStageConfig
{
    uint64_t numSRV;
    uint64_t numSampler;
    uint64_t numCBV;
};

struct RootSigConfig
{
    ShaderStageConfig pixelStage;
    ShaderStageConfig vertexStage;
    uint64_t numUAV;
};

CComPtr<ID3D12RootSignature> CreateRootsigFromConfig(ID3D12Device* device, const RootSigConfig& config)
{
    struct RootParam
    {
        D3D12_SHADER_VISIBILITY visibility;
        D3D12_DESCRIPTOR_RANGE_TYPE type;
        uint64_t num;
        D3D12_DESCRIPTOR_RANGE_FLAGS flags;
    };

    static constexpr uint64_t numParams = 7;

    RootParam params[numParams] = {
        { D3D12_SHADER_VISIBILITY_PIXEL,  D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     config.pixelStage.numSRV,      D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE },
        { D3D12_SHADER_VISIBILITY_PIXEL,  D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     config.pixelStage.numCBV,      D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE },
        { D3D12_SHADER_VISIBILITY_PIXEL,  D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, config.pixelStage.numSampler,  D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE },
        { D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     config.vertexStage.numSRV,     D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE },
        { D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     config.vertexStage.numCBV,     D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE },
        { D3D12_SHADER_VISIBILITY_VERTEX, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, config.vertexStage.numSampler, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE },
        { D3D12_SHADER_VISIBILITY_ALL,    D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     config.numUAV,                 D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE },
    };

    CD3DX12_ROOT_PARAMETER1 rootParameters[numParams];
    CD3DX12_DESCRIPTOR_RANGE1 range[numParams];
    for (uint64_t index = 0; index < numParams; ++index)
    {
        RootParam& param = params[index];
        range[index].Init(param.type, param.num, 0, 0, param.flags);
        rootParameters[index].InitAsDescriptorTable(1, &range[index], param.visibility);
    }

    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(numParams, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CComPtr<ID3DBlob> signatureBytes;
    CComPtr<ID3DBlob> error;
    DCHECK_COM(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signatureBytes, &error));

    CComPtr<ID3D12RootSignature> rootSig;
    DCHECK_COM(device->CreateRootSignature(0, signatureBytes->GetBufferPointer(), signatureBytes->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

    return rootSig;
}

class DeviceConfig
{
public:
    DeviceConfig(ID3D12Device* device)
        : mSRVAllocator(device, 8, 10000, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
        , mSamplerAllocator(device, 8, 256, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
    {
        RootSigConfig defaultConfig = {
            { 8, 8, 8 },
            { 8, 8, 8 },
            8
        };
        mRootSig = CreateRootsigFromConfig(device, defaultConfig);
    }
    ID3D12RootSignature* GetDefaultRootSig() { return mRootSig.p; };
    DescriptorHeapAllocator& GetSamplerAllocator() { return mSamplerAllocator; };
    DescriptorHeapAllocator& GetSRVAllocator() { return mSRVAllocator; };

private:
    CComPtr<ID3D12RootSignature> mRootSig;
    DescriptorHeapAllocator mSamplerAllocator;
    DescriptorHeapAllocator mSRVAllocator;
};
