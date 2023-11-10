using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class AmbientLightDef : LightDef
    {
        public AmbientLightDef()
        {
            ComponentType = typeof(AmbientLight);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(AmbientLight) && !ComponentType.IsSubclassOf(typeof(AmbientLight)))
            {
                Logger.IcarianError($"Ambient Light Def Invalid ComponentType: {ComponentType}");
            }
        }
    }
}