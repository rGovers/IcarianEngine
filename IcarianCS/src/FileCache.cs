// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Mod;
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace IcarianEngine
{
    public static class FileCache
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint ExistingFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static uint CachedFile(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static byte[] ReadFileData(string a_path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void WriteFileData(string a_path, byte[] a_data, uint a_writeFile, uint a_pinFile);

        public static bool FileExists(string a_path)
        {
            return ExistingFile(a_path) != 0;
        }

        /// <summary>
        /// Whether a file is loaded in the FileCache
        /// </summary>
        /// <returns>If the file is loaded in the FileCache</returns>
        public static bool IsFileCached(string a_path)
        {
            return CachedFile(a_path) != 0;
        }

        /// <summary>
        /// Loads file data from the FileCache
        /// </summary>
        /// <param name="a_path">The path to the Asset</param>
        /// <returns>The file data. Null on failure</returns>
        public static byte[] LoadData(string a_path)
        {
            return ReadFileData(a_path);
        }

        /// <summary>
        /// Writes data to the FileCache. Ignores current size of the FileCache
        /// </summary>
        /// <param name="a_path">The path to the Asset</param>
        /// <param name="a_modID">The <see cref="IcarianEngine.Mod.IcarianAssembly" /> to write the Asset to</param>
        /// <param name="a_data">The data to write</param>
        /// <param name="a_writeFile">Write to the cache and file or only cache</param>
        /// <param name="a_pinFile">Stops the file from unloading from the FileCache</param>
        public static void WriteData(string a_path, string a_modID, byte[] a_data, bool a_writeFile, bool a_pinFile)
        {
            string path = a_path;
            if (!Application.IsEditor)
            {
                path = ModControl.GetAssetPath(a_path, a_modID);
            }

            uint writeFile = 0;
            if (a_writeFile)
            {
                writeFile = 1;
            }

            uint pinFile = 0;
            if (a_pinFile)
            {
                pinFile = 1;
            }

            WriteFileData(path, a_data, writeFile, pinFile);
        }
    }
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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
