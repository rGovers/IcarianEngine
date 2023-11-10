using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

#include "EngineLightInteropStructures.h"

namespace IcarianEngine.Rendering.Lighting
{
    public abstract class Light : Component
    {
        /// <summary>
        /// Returns the LightType of the Light.
        /// </summary>
        public abstract LightType LightType
        {
            get;
        }

        /// <summary>
        /// Returns the RenderLayer of the Light.
        /// </summary>
        /// Bitmask of the layers the light will affect.
        public abstract uint RenderLayer
        {
            get;
            set;
        }

        /// <summary>
        /// Color of the Light.
        /// </summary>
        public abstract Color Color
        {
            get;
            set;
        }
        /// <summary>
        /// Intensity of the Light.
        /// </summary>
        public abstract float Intensity
        {
            get;
            set;
        }

        /// <summary>
        /// ShadowMap of the Light.
        /// </summary>
        public abstract IEnumerable<IRenderTexture> ShadowMaps
        {
            get;
        }

        /// <summary>
        /// Returns the Definition used to create the Light.
        /// </summary>
        public LightDef LightDef
        {
            get
            {
                return Def as LightDef;
            }
        }
    }
}