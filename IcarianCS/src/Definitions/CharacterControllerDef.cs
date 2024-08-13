// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using IcarianEngine.Physics;

namespace IcarianEngine.Definitions
{
    public class CharacterControllerDef : ComponentDef
    {
        public float Mass = 100.0f;
        public float SlopeAngle = 1.0f;
        public Vector3 Up = new Vector3(0.0f, -1.0f, 0.0f);
        public CollisionShapeDef CollisionShape = null;

        public CharacterControllerDef()
        {
            ComponentType = typeof(CharacterController);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(CharacterController) && !ComponentType.IsSubclassOf(typeof(CharacterController)))
            {
                Logger.IcarianError($"CharacterControllerDef {DefName} Invalid ComponentType: {ComponentType}");

                return;
            }

            if (CollisionShape == null)
            {
                Logger.IcarianWarning($"CharacterControllerDef {DefName} null CollisionShape");

                return;
            }
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