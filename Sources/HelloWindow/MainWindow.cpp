#include <memory>

#include <DebugCheck.h>
#include <ShaderCompiler.h>
#include <DeviceConfig.h>
#include <ResourceUploader.h>
#include <CmdListRecording.h>

#include <SDL.h>
#include <d3d12.h>
#include <d3dx12.h>

#include "WindowDX12.h"

class TriangleRenderer : public ICmdListSubRecorder
{
private:
    CComPtr<ID3D12PipelineState> mTrianglePso;
    CComPtr<ID3D12Resource> mVertexResource;

    D3D12_VERTEX_BUFFER_VIEW mVertexView{};

    CComPtr<ID3D12Device> mDevice;
    std::shared_ptr<DeviceConfig> mDeviceConfig;

public:
    TriangleRenderer(CComPtr<ID3D12Device> device, std::shared_ptr<DeviceConfig> deviceConfig)
        : mDevice(device)
        , mDeviceConfig(deviceConfig)
    {}
    void CreateResources(IResourceUploader* resourceUploader, ShaderCompiler* shaderCompiler)
    {
        auto testShaderSource = shaderCompiler->GetShader(L"shaders/TestShader.hlsl");
        auto vsBytecode = shaderCompiler->Compile(L"TestShader.hlsl", testShaderSource, ShaderType::VS, L"MainVS");
        auto psBytecode = shaderCompiler->Compile(L"TestShader.hlsl", testShaderSource, ShaderType::PS, L"MainPS");
        D3D12_INPUT_ELEMENT_DESC geometryFormat[1] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.pRootSignature = mDeviceConfig->GetDefaultRootSig();
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
        DCHECK_COM(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mTrianglePso)));

        // vertex data
        float vertexData[] = {
            0.0f, -1.0f, 1.0f,
            -1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
        };

        auto vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));
        auto vertexHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        DCHECK_COM(mDevice->CreateCommittedResource(&vertexHeapProperties, D3D12_HEAP_FLAG_NONE,
            &vertexBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mVertexResource)));
        resourceUploader->UploadImmidiate(mVertexResource, vertexData, sizeof(vertexData));

        mVertexView.BufferLocation = mVertexResource->GetGPUVirtualAddress();
        mVertexView.SizeInBytes = sizeof(vertexData);
        mVertexView.StrideInBytes = sizeof(float) * 3;
    }
    // ICmdListSubRecorder
    void Record(ID3D12GraphicsCommandList* destination) override
    {
        D3D12_RESOURCE_BARRIER barrier;

        destination->IASetVertexBuffers(0, 1, &mVertexView);
        destination->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        destination->SetGraphicsRootSignature(mDeviceConfig->GetDefaultRootSig());

        //float resolution[] = { (float)dx12Window->GetWindowRect().right, (float)dx12Window->GetWindowRect().bottom };
        //destination->SetGraphicsRoot32BitConstants(0, 2, resolution, 0);

        destination->SetPipelineState(mTrianglePso);
        destination->DrawInstanced(3, 1, 0, 0);
    }
};

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
    auto scheduler = std::make_shared<CmdListScheduler>(device, dx12Window->GetRenderQueue(), backbufferCount);
    auto shaderCompiler = std::make_shared<ShaderCompiler>();
    auto deviceConfig = std::make_shared<DeviceConfig>(device);
    auto resourceUploader = std::make_shared<SimpleResourceUploader>(device4);

    TriangleRenderer triangleRenderer(device, deviceConfig);
    triangleRenderer.CreateResources(resourceUploader.get(), shaderCompiler.get());

    LambdaSubRecorder beginRender([dx12Window](ID3D12GraphicsCommandList* destination)
    {
        destination->RSSetScissorRects(1, &dx12Window->GetWindowRect());
        destination->RSSetViewports(1, &dx12Window->GetViewport());

        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Window->GetCurrentBackbuffer(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        destination->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dx12Window->GetCurrentRtvHandle();
        destination->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

        float clearRGB[] = { 1.0f, 0.3f, 0.1f, 1.0f };
        destination->ClearRenderTargetView(rtvHandle, clearRGB, 0, nullptr);
    });

    LambdaSubRecorder endRender([dx12Window](ID3D12GraphicsCommandList* destination)
    {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(dx12Window->GetCurrentBackbuffer(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        destination->ResourceBarrier(1, &barrier);
    });

    std::vector<ICmdListSubRecorder*> renderSequence = { &beginRender, &triangleRenderer, &endRender };
    CmdListRecorder recorder(device, renderSequence);

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

        scheduler->NewFrame(dx12Window->GetCurrentFrameIndex());
        scheduler->Submit(&recorder);

        dx12Window->SubmitNextFrame();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
