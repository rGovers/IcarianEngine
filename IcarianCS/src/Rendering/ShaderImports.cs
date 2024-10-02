// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "GlobalImports.h"
#include "PixelImports.h"

namespace IcarianEngine.Rendering
{
    internal partial class ShaderImports
    {
        internal static void Init()
        {
            // May move this to C++ for the default imports need to think about it
            VertexShader.AddImport("Maths", MathsImportShader);
            PixelShader.AddImport("Maths", MathsImportShader);

            PixelShader.AddImport("Camera", CameraImportShader);

            PixelShader.AddImport("PBR", PBRImportShader);
            PixelShader.AddImport("Lighting", LightingImportShader);
            PixelShader.AddImport("DirectionalLight", DirectionalLightImportShader);
            PixelShader.AddImport("PointLight", PointLightImportShader);
            PixelShader.AddImport("SpotLight", SpotLightImportShader);
        }
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.