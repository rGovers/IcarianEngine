using IcarianEngine.Rendering.Lighting;

namespace IcarianEngine.Definitions
{
    public class PointLightDef : LightDef
    {
        public float Radius = 1.0f;

        public PointLightDef()
        {
            ComponentType = typeof(PointLight);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(PointLight) && !ComponentType.IsSubclassOf(typeof(PointLight)))
            {
                Logger.IcarianError($"Point Light Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}