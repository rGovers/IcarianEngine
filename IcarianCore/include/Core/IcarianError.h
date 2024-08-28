// Icarian Engine - C# Game Engine
//
// License at end of file.

#pragma once

#include "Core/IcarianDefer.h"

// This is not free there is a cost with the macros but it is pretty minimal still would not use it if you do not need it
// Mostly exists because error cleanup is a pain and not a fan of the goto return pattern
#define IERRBLOCK bool _iErrVal = false
#define ITRIGGERERR _iErrVal = true; return
#define ITRIGGERERRRET(val) _iErrVal = true; return val
#define IERRCHECK(cond) do { if (!(cond)) { ITRIGGERERR; } } while(0)
#define IERRCHECKRET(cond, val) do { if (!(cond)) { ITRIGGERERRRET(val); } } while(0)
#define IERRDEFER(code) IDEFER(if (_iErrVal) { code; })

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
