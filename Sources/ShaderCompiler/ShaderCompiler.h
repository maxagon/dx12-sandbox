#pragma once

#include <string>
#include <memory>

#include <atlcomcli.h>
#include <dxcapi.h>

enum class ShaderType
{
    VS,
    PS
};

class ShaderCompiler
{
public:
    ShaderCompiler();
    CComPtr<IDxcBlobEncoding> GetShader(const std::wstring& shader);
    CComPtr<IDxcBlob> Compile(const std::wstring& shaderName, CComPtr<IDxcBlobEncoding> shaderBlob, ShaderType type, const std::wstring& mainFunc);
private:
    CComPtr<IDxcUtils> mUtils;
    CComPtr<IDxcCompiler3> mCompiler;
    CComPtr<IDxcIncludeHandler> mIncludeHandler;
};
