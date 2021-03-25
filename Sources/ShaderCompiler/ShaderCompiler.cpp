#include "ShaderCompiler.h"

#include <DebugCheck.h>
#include <fmt/core.h>
#include <filesystem>

ShaderCompiler::ShaderCompiler()
{
    DCHECK_COM(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&mUtils)));
    DCHECK_COM(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mCompiler)));
    DCHECK_COM(mUtils->CreateDefaultIncludeHandler(&mIncludeHandler));
}

CComPtr<IDxcBlobEncoding> ShaderCompiler::GetShader(const std::wstring& shader)
{
    static std::filesystem::path exeFolder;
    if (exeFolder.empty())
    {
        WCHAR winExePath[MAX_PATH];
        GetModuleFileNameW(NULL, winExePath, MAX_PATH);
        exeFolder.assign(winExePath);
        exeFolder = exeFolder.remove_filename();
    }

    CComPtr<IDxcBlobEncoding> source = nullptr;
    std::filesystem::path fullPath = exeFolder.append(shader);
    DCHECK_COM(mUtils->LoadFile(fullPath.c_str(), nullptr, &source));
    return source;
}

CComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& shaderName, CComPtr<IDxcBlobEncoding> shaderBlob, ShaderType type, const std::wstring& mainFunc)
{
    DxcBuffer source;
    source.Ptr = shaderBlob->GetBufferPointer();
    source.Size = shaderBlob->GetBufferSize();
    source.Encoding = DXC_CP_ACP;

    std::wstring shaderTypeArg;
    switch (type)
    {
    case ShaderType::VS:
        shaderTypeArg = L"vs_6_4";
        break;
    case ShaderType::PS:
        shaderTypeArg = L"ps_6_4";
        break;
    default:
        break;
    }

    const wchar_t* compilerArgs[] =
    {
        shaderName.c_str(),
        L"-E", mainFunc.c_str(),
        L"-T", shaderTypeArg.c_str(),
        L"-Zi",
    };

    CComPtr<IDxcResult> results;
    DCHECK_COM(mCompiler->Compile(
        &source,
        compilerArgs,
        _countof(compilerArgs),
        mIncludeHandler,
        IID_PPV_ARGS(&results)
    ));

    CComPtr<IDxcBlobUtf8> errors = nullptr;
    DCHECK_COM(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if (errors != nullptr && errors->GetStringLength() != 0)
    {
        DebugPrint(L"Compile shader: {}", shaderName);
        DebugPrint("{}", errors->GetStringPointer());
    }

    HRESULT status;
    DCHECK_COM(results->GetStatus(&status));
    if (FAILED(status))
    {
        DebugPrint("Shader compilation failed!");
    }
    DCHECK_COM(status);

    CComPtr<IDxcBlob> shaderBinary = nullptr;
    results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBinary), nullptr);
    return shaderBinary;
}
