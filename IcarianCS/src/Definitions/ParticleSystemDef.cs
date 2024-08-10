using IcarianEngine.Maths;
using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class ParticleSystemDef : ComponentDef
    {
        /// <summary>
        /// Start simulation on creation
        /// </summary>
        public bool AutoPlay = true;

        /// <summary>
        /// The emitter type for the simulation
        /// </summary>
        public ParticleEmitterType EmitterType = ParticleEmitterType.Point;

        /// <summary>
        /// The radius of the emitter
        /// <summary>
        public float EmitterRadius = 1.0f;

        /// <summary>
        /// The bounds of the emitter
        /// <summary>
        public Vector3 EmitterBounds;

        /// <summary>
        /// The maximum particles of the system
        /// </summary>
        public uint MaxParticles;

        /// <summary>
        /// The RenderLayer of the system
        /// </summary>
        [EditorBitfield]
        public uint RenderLayer;

        /// <summary>
        /// The gravity to apply to the particles
        /// </summary>
        public Vector3 Gravity = new Vector3(0.0f, 9.807f, 0.0f);

        /// <summary>
        /// The color of the particles
        /// </summary>
        public Color Color;

        /// <summary>
        /// Is a burst particle system
        /// </summary>
        public bool Burst;

        /// <summary>
        /// Constructor for <cref="IcarianEngine.Definitions.ParticleSystemDef"/>
        /// <summary>
        public ParticleSystemDef()
        {
            ComponentType = typeof(ParticleSystem);
        }

        /// <summary>
        /// Called after the def is loaded to resolve any data.
        /// </summary>
        public override void PostResolve()
        {
            base.PostResolve();

            if (!ComponentType.IsSubclassOf(typeof(ParticleSystem)))
            {
                Logger.IcarianError($"Particle System Def Invalid ComponentType: {ComponentType}");

                return;
            }

            if (EmitterType == ParticleEmitterType.Null)
            {
                Logger.IcarianWarning($"Particle System Def No emitter type");
            }
        }
    }
}