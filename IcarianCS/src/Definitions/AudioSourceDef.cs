using IcarianEngine.Audio;

namespace IcarianEngine.Definitions
{
    public class AudioSourceDef : ComponentDef
    {   
        /// <summary>
        /// The AudioClip to play.
        /// </summary>
        [EditorTooltip("The AudioClip to play"), EditorPathString]
        public string AudioClipPath;
        /// <summary>
        /// Whether or not the AudioClip should loop.
        /// </summary>
        [EditorTooltip("Whether or not the AudioClip should loop")]
        public bool Loop = false;
        /// <summary>
        /// Whether or not the AudioClip should play on creation.
        /// </summary>
        [EditorTooltip("Whether or not the AudioClip should play on creation")]
        public bool PlayOnCreation = false;

        public AudioSourceDef()
        {
            ComponentType = typeof(AudioSource);
        }
    }
}