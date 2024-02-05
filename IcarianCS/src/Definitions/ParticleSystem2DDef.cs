using IcarianEngine.Rendering;

namespace IcarianEngine.Definitions
{
    public class ParticleSystem2DDef : ParticleSystemDef
    {
        public ParticleSystem2DDef()
        {
            ComponentType = typeof(ParticleSystem2D);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(ParticleSystem2D) && !ComponentType.IsSubclassOf(typeof(ParticleSystem2D)))
            {
                Logger.IcarianError($"Particle System 2D Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}