using IcarianEngine.Maths;
using IcarianEngine.Rendering;

namespace IcarianEngine
{
    public enum PrimitiveType
    {
        Cube
    };

    public static class PrimitiveGenerator
    {
        public static Model CreateCube()
        {
            Vertex[] vertices = 
            {
                // 0
                new Vertex() 
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Left,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Down,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Backward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero

                },

                // 3
                new Vertex() 
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Right,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Down,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Backward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 6
                new Vertex() 
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Left,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Down,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Forward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 9
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Right,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Down,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, -1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Forward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 12
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Left,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Up,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Backward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 15
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Right,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Up,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, -1.0f, 1.0f),
                    Normal = Vector3.Backward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 18
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Left,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Up,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(-1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Forward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },

                // 21
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Right,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Up,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                },
                new Vertex()
                {
                    Position = new Vector4(1.0f, 1.0f, 1.0f, 1.0f),
                    Normal = Vector3.Forward,
                    Color = Vector4.One,
                    TexCoords = Vector2.Zero
                }
            };
            uint[] indices = 
            {
                0,  6,  18, 0,  18, 12,
                1,  4,  10, 1,  10, 7,
                2,  17, 5,  2,  14, 17,
                3,  21, 9,  3,  15, 21,
                8,  11, 23, 8,  23, 20,
                13, 22, 16, 13, 19, 22
            };

            return Model.CreateModel(vertices, indices);
        }   

        public static Model CreatePrimitive(PrimitiveType a_primitiveType)
        {
            switch (a_primitiveType)
            {
            case PrimitiveType.Cube:
            {
                return CreateCube();
            }
            }

            return null;   
        }
    }
}