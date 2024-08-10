using IcarianEngine.Maths;
using IcarianEngine.Rendering;
using System;

namespace IcarianEngine.Definitions
{
    public struct RenderTextureData
    {
        /// <summary>
        /// The width of the <see cref="IcarianEngine.Rendering.IRenderTexture" />
        /// </summary>
        public uint Width;
        /// <summary>
        /// The height of the <see cref="IcarianEngine.Rendering.IRenderTexture" />
        /// </summary>
        public uint Height;
        /// <summary>
        /// The number of textures in <see cref="IcarianEngine.Rendering.IRenderTexture" />
        /// </summary>
        public uint Count;
        /// <summary>
        /// If the <see cref="IcarianEngine.Rendering.IRenderTexture" /> is HDR
        /// </summary>
        public bool HDR;
    }

    public class CameraDef : ComponentDef
    {
        /// <summary>
        /// <see cref="IcarianEngine.Rendering.Viewport" /> to determine the portion of the screen rendered to
        /// </summary>
        [EditorTooltip("Viewport to determine the portion of the screen rendered to.")]
        public Viewport Viewport = new Viewport()
        {
            Position = Vector2.Zero,
            Size = Vector2.One,
            MinDepth = 0.0f,
            MaxDepth = 1.0f
        };
        /// <summary>
        /// Field of View for the camera
        /// </summary>
        [EditorAngle, EditorRange(0.1f, Mathf.PI), EditorTooltip("Field of View for the camera.")]
        public float FOV = (float)(Math.PI * 0.45f);
        /// <summary>
        /// Near clipping plane for the camera
        /// </summary>
        [EditorTooltip("Near clipping plane for the camera.")]
        public float Near = 0.1f;
        /// <summary>
        /// Far clipping plane for the camera
        /// </summary>
        [EditorTooltip("Far clipping plane for the camera.")]
        public float Far = 100.0f;
        /// <summary>
        /// RenderLayer for the camera
        /// </summary>
        [EditorBitfield, EditorTooltip("Renders objects of the same render layer.")]
        public uint RenderLayer = 0b1;

        public RenderTextureData RenderTexture = new RenderTextureData()
        {
            Width = uint.MaxValue,
            Height = uint.MaxValue,
            Count = 1,
            HDR = false
        };

        public CameraDef()
        {
            ComponentType = typeof(Camera);
        }

        public override void PostResolve()
        {
            base.PostResolve();

            if (ComponentType != typeof(Camera) && !ComponentType.IsSubclassOf(typeof(Camera)))
            {
                Logger.IcarianError($"Camera Def Invalid ComponentType: {ComponentType}");

                return;
            }
        }
    }
}