using IcarianEngine.Definitions;

namespace IcarianEngine.Rendering.Animation
{
    // Using animation controllers
    // Probably want to implement state machine version down the line
    // By having it working with objects down the line can potentially use it to animate UI down the line
    // By using a controller should allow it to be controlled by anything instead of just state machines
    public abstract class AnimationController
    {
        public AnimationControllerDef ControllerDef
        {
            get;
            internal set;
        }

        public virtual void Init() { }

        public abstract bool Update(Animator a_animator, double a_deltaTime);
        public abstract void UpdateObject(Animator a_animator, string a_object, double a_deltaTime);
    }
}