using IcarianEngine.Definitions;
using IcarianEngine.Maths;
using System.Collections.Generic;

namespace IcarianEngine.Rendering.Lighting
{
    public abstract class ShadowLight : Light
    {
        /// <summary>
        /// The Definition used to create the ShadowLight
        /// </summary>
        public ShadowLightDef ShadowLightDef
        {
            get
            {
                return Def as ShadowLightDef;
            }
        }

        /// <summary>
        /// Shadow Map of the ShadowLight
        /// </summary>
        public abstract IEnumerable<IRenderTexture> ShadowMaps
        {
            get;
        }

        /// <summary>
        /// Shadow bias of the ShadowLight
        /// </summary>
        public abstract Vector2 ShadowBias
        {
            get;
            set;
        }

        /// <summary>
        /// The depth bias for the ShadowLight
        /// </summary>
        public float ShadowBiasConstant
        {
            get
            {
                return ShadowBias.X;
            }
            set
            {
                Vector2 val = ShadowBias;

                val.X = value;

                ShadowBias = val;
            }
        }

        /// <summary>
        /// The depth slope for the ShadowLight
        /// </summary>
        public float ShadowBiasSlope
        {
            get
            {
                return ShadowBias.Y;
            }
            set
            {
                Vector2 val = ShadowBias;

                val.Y = value;

                ShadowBias = val;
            }
        }
    }
}