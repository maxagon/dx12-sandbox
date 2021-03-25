#include <memory>

#include <DebugCheck.h>
#include <ShaderCompiler.h>

#include <SDL.h>
#include <d3d12.h>
#include <d3dx12.h>

#include "WindowDX12.h"
#include "CommandAllocatorPool.h"

CComPtr<ID3D12Resource> CreateUploadResource(ID3D12Device* device, void* data, uint64_t size)
{
    auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    CComPtr<ID3D12Resource> resource;
    DCHECK_COM(device->CreateCommittedResource(
        &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource
    )));

    void* resourceData = nullptr;
    DCHECK_COM(resource->Map(0, nullptr, &resourceData));
    memcpy(resourceData, data, size);
    resource->Unmap(0, nullptr);
    return resource;
}

int SDL_main(int argc, char *argv[])
{
    DCHECK(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == 0);

    constexpr int32_t screenSizeX = 1024;
    constexpr int32_t screenSizeY = 768;
    SDL_Window* window = SDL_CreateWindow(
        "hello",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenSizeX, screenSizeY,
        SDL_WINDOW_SHOWN
        );
    DCHECK(window);


    CComPtr<ID3D12Device> device;
    DCHECK_COM(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));


    CComPtr<ID3D12Device4> device4;
    DCHECK_COM(device->QueryInterface(IID_PPV_ARGS(&device4)));

    static constexpr uint32_t backbufferCount = 2;
    auto dx12Window = std::make_shared<WindowDX12>(GetActiveWindow(), device, backbufferCount);
    auto allocatorPool = std::make_shared<CommandAllocatorPool>(device, backbufferCount);
    auto shaderCompiler = std::make_shared<ShaderCompiler>();

    // root sig
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(3, 0, 0);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    CComPtr<ID3DBlob> signatureBytes;
    D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signatureBytes, nullptr);

    CComPtr<ID3D12RootSignature> rootSig;
    DCHECK_COM(device->CreateRootSignature(0, signatureBytes->GetBufferPointer(), signatureBytes->GetBufferSize(), IID_PPV_ARGS(&rootSig)));

    // pso
    auto testShaderSource = shaderCompiler->GetShader(L"shaders/TestShader.hlsl");
    auto vsBytecode = shaderCompiler->Compile(L"TestShader.hlsl", testShaderSource, ShaderType::VS, L"MainVS");
    auto psBytecode = shaderCompiler->Compile(L"TestShader.hlsl", testShaderSource, ShaderType::PS, L"MainPS");

    D3D12_INPUT_ELEMENT_DESC geometryFormat[1] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    CComPtr<ID3D12PipelineState> testPSO;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSig;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBytecode->GetBufferPointer(), vsBytecode->GetBufferSize());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBytecode->GetBufferPointer(), psBytecode->GetBufferSize());
    psoDesc.InputLayout = { geometryFormat, _countof(geometryFormat) };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = false;
    psoDesc.DepthStencilState.StencilEnable = false;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    DCHECK_COM(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&testPSO)));

    // vertex data
    float vertexData[] = {
        0.0f, -1.0f, 1.0f,
        -1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    CComPtr<ID3D12Fence> uploadSync;
    DCHECK_COM(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadSync)));

    CComPtr<ID3D12CommandQueue> uploadQueue;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    DCHECK_COM(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&uploadQueue)));

    CComPtr<ID3D12Resource> vertexDataUpload = CreateUploadResource(device, vertexData, sizeof(vertexData));

    auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));
    auto vertexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    CComPtr<ID3D12Resource> vertexResource;
    DCHECK_COM(device->CreateCommittedResource(&vertexHeapProperties, D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vertexResource)));

    D3D12_VERTEX_BUFFER_VIEW vertexView{};
    vertexView.BufferLocation = vertexResource->GetGPUVirtualAddress();
    vertexView.SizeInBytes = sizeof(vertexData);
    vertexView.StrideInBytes = sizeof(float) * 3;

    CComPtr<ID3D12CommandAllocator> uploadAllocator;
    DCHECK_COM(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&uploadAllocator)));

    CComPtr<ID3D12GraphicsCommandList> uploadCmd;
    DCHECK_COM(device4->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&uploadCmd)));
    uploadCmd->Reset(uploadAllocator, nullptr);
    uploadCmd->CopyResource(vertexResource.p, vertexDataUpload);
    DCHECK_COM(uploadCmd->Close());

    ID3D12CommandList* uploadLists[] = { uploadCmd.p };
    uploadQueue->ExecuteCommandLists(_countof(uploadLists), uploadLists);
    DCHECK_COM(uploadQueue->Signal(uploadSync, 1));

    if (uploadSync->GetCompletedValue() < 0)
    {
        auto uploadEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        DCHECK_COM(uploadSync->SetEventOnCompletion(1, uploadAllocator));
        WaitForSingleObject(uploadEvent, INFINITE);
        CloseHandle(uploadEvent);
    }

    bool isFirstFrame = false;

    // render loop
    while (true)
    {
        SDL_Event sdlEvent{};
        if (SDL_PollEvent(&sdlEvent))
        {
            if (sdlEvent.type == SDL_QUIT)
            {
                break;
            }
        }
        dx12Window->WaitForNextFrame();

        ID3D12CommandAllocator* cmdAllocForFrame = allocatorPool->GetAllocatorForFrame(dx12Window->GetCurrentFrameIndex());

        CComPtr<ID3D12GraphicsCommandList> cmdList;
        device4->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&cmdList));
        cmdList->Reset(cmdAllocForFrame, nullptr);

        cmdList->RSSetScissorRects(1, &dx12Window->GetWindowRect());
        cmdList->RSSetViewports(1, &dx12Window->GetViewport());
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12Window->GetCurrentRtvHandle();

        D3D12_RESOURCE_BARRIER barrier;

        if(isFirstFrame)
        {
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(vertexResource.p,
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            cmdList->ResourceBarrier(1, &barrier);
            isFirstFrame = false;
        }

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Window->GetCurrentBackbuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmdList->ResourceBarrier(1, &barrier);
        cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
        float clearRGB[] = { 1.0f, 0.3f, 0.1f, 1.0f };
        cmdList->ClearRenderTargetView(rtvHandle, clearRGB, 0, nullptr);

        cmdList->IASetVertexBuffers(0, 1, &vertexView);
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->SetGraphicsRootSignature(rootSig);

        float resolution[] = { (float)dx12Window->GetWindowRect().right, (float)dx12Window->GetWindowRect().bottom };
        cmdList->SetGraphicsRoot32BitConstants(0, 2, resolution, 0);

        cmdList->SetPipelineState(testPSO);
        cmdList->DrawInstanced(3, 1, 0, 0);

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Window->GetCurrentBackbuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        cmdList->ResourceBarrier(1, &barrier);
        DCHECK_COM(cmdList->Close());

        ID3D12CommandList* commandLists[] = { cmdList.p };
        dx12Window->GetRenderQueue()->ExecuteCommandLists(1, commandLists);

        dx12Window->SubmitNextFrame();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
