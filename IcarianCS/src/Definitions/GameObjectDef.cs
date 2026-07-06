// Icarian Engine - C# Game Engine
// 
// License at end of file.

using IcarianEngine.Maths;
using System;
using System.Collections.Generic;

namespace IcarianEngine.Definitions
{
    public class GameObjectDef : Def
    {
        /// <summary>
        /// Type of <see cref="IcarianEngine.GameObject" /> to create
        /// </summary>
        [EditorTooltip("Type of GameObject to create"), EditorTypeInherits(typeof(GameObject), true)]
        public Type ObjectType = typeof(GameObject);

        /// <summary>
        /// Name to give the spawned <see cref="IcarianEngine.GameObject" />
        /// </summary>
        [EditorTooltip("Name to give the spawned GameObject")]
        public string Name;

        /// <summary>
        /// <see cref="IcarianEngine.GameObject" /> local position offset
        /// </summary>
        [EditorTooltip("GameObject local position offset")]
        public Vector3 Translation = Vector3.Zero;
        /// <summary>
        /// <see cref="IcarianEngine.GameObject" /> local rotation offset
        /// </summary>
        [EditorTooltip("GameObject local rotation offset")]
        public Quaternion Rotation = Quaternion.Identity;
        /// <summary>
        /// <see cref="IcarianEngine.GameObject" /> local scale offset
        /// </summary>
        [EditorTooltip("GameObject local scale offset")]
        public Vector3 Scale = Vector3.One;

        /// <summary>
        /// List of <see cref="IcarianEngine.Component" />s the <see cref="IcarianEngine.GameObject" /> is composed of
        /// </summary>
        [EditorTooltip("List of Components the GameObject is composed of")]
        public List<ComponentDef> Components = new List<ComponentDef>();

        /// <summary>
        /// List of children of the <see cref="IcarianEngine.GameObject" />
        /// </summary>
        [EditorTooltip("List of children of the GameObject")]
        public List<GameObjectDef> Children = new List<GameObjectDef>();

        /// <summary>
        /// Called after all the <see cref="IcarianEngine.Definitions.Def" /> are loaded and resolved
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (ObjectType != typeof(GameObject) && !ObjectType.IsSubclassOf(typeof(GameObject)))
            {
                Logger.IcarianError($"Game Object Def Invalid ObjectType: {ObjectType}");

                return;
            }
        }

        /// <summary>
        /// Gets a <see cref="IcarianEngine.Definition.ComponentDef" /> of Type T from the GameObjectDef
        /// </summary>
        /// <returns>The <see cref="IcarianEngine.Definition.ComponentDef" /> of Type T. Null on failure</returns>
        public T GetComponentDef<T>() where T : ComponentDef
        {
            if (Components == null)
            {
                return null;
            }

            foreach (ComponentDef c in Components)
            {
                if (c is T val)
                {
                    return val;
                }
            }

            return null;
        }
    }
}

// MIT License
// 
// Copyright (c) 2026 River Govers
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