// Icarian Engine - C# Game Engine
// 
// License at end of file.

using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace IcarianEngine.Rendering
{
    /// @cond INTERNAL

    internal static class RenderTextureCmd
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GenerateRenderTexture(uint a_count, uint a_width, uint a_height, uint a_depth, uint a_hdr, uint a_channelCount);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GenerateRenderTextureD(uint a_count, uint a_width, uint a_height, uint a_depthHandle, uint a_hdr, uint a_channelCount);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void DestroyRenderTexture(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint HasDepth(uint a_addr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetWidth(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetHeight(uint a_addr);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Resize(uint a_addr, uint a_width, uint a_height);

        static Dictionary<uint, IRenderTexture> s_renderTextureTable = new Dictionary<uint, IRenderTexture>();
        static Dictionary<uint, IRenderTexture> s_depthRenderTextureTable = new Dictionary<uint, IRenderTexture>();

        internal static void PushRenderTexture(uint a_addr, IRenderTexture a_renderTexture)
        {
            if (!s_renderTextureTable.ContainsKey(a_addr))
            {
                s_renderTextureTable.Add(a_addr, a_renderTexture);
            }
            else
            {
                Logger.IcarianWarning($"RenderTexture exists at {a_addr}");

                s_renderTextureTable[a_addr] = a_renderTexture;
            }
        } 
        internal static void RemoveRenderTexture(uint a_addr)
        {
            s_renderTextureTable.Remove(a_addr);
        }

        internal static IRenderTexture GetRenderTexture(uint a_addr)
        {
            if (a_addr == uint.MaxValue)
            {
                return null;
            }

            return s_renderTextureTable[a_addr];
        }

        internal static uint GetTextureAddr(IRenderTexture a_renderTexture)
        {
            if (a_renderTexture != null)
            {
                if (a_renderTexture is RenderTexture rVal)
                {
                    return rVal.BufferAddr;
                }
                else if (a_renderTexture is MultiRenderTexture mVal)
                {
                    return mVal.BufferAddr;
                }
            }

            return uint.MaxValue;
        }
    }


    /// @endcond
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