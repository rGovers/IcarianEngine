// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering.Animation
{
    // Using animation controllers
    // Probably want to implement state machine version down the line
    // By having it working with objects down the line can potentially use it to animate UI down the line
    // By using a controller should allow it to be controlled by anything instead of just state machines
    public abstract class AnimationController
    {
        public AnimationControllerDef ControllerDef
        {
            get;
            internal set;
        }

        public virtual void Init() { }

        public abstract bool Update(Animator a_animator, double a_deltaTime);
        public abstract void UpdateObject(Animator a_animator, string a_object, double a_deltaTime);
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