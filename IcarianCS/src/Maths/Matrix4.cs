namespace IcarianEngine.Maths
{
    public struct Matrix4
    {
        public static readonly Matrix4 Identity = new Matrix4(1.0f);

        float[] m_data;

        /// <summary>
        /// Returns the row at the specified index
        /// </summary>
        public Vector4 this[int a_key]
        {
            get
            {
                int offset = a_key * 4;

                return new Vector4(m_data[offset + 0], m_data[offset + 1], m_data[offset + 2], m_data[offset + 3]);
            }
            set
            {
                int offset = a_key * 4;

                m_data[offset + 0] = value.X;
                m_data[offset + 1] = value.Y;
                m_data[offset + 2] = value.Z;
                m_data[offset + 3] = value.W;
            }
        }

        /// <summary>
        /// Creates a new matrix with the specified values
        /// </summary>
        public Matrix4(float a_val) : this(a_val, 0.0f, 0.0f, 0.0f,
                                           0.0f, a_val, 0.0f, 0.0f,
                                           0.0f, 0.0f, a_val, 0.0f,
                                           0.0f, 0.0f, 0.0f, a_val)
        {

        }
        /// <summary>
        /// Creates a new matrix with the specified values
        /// </summary>
        public Matrix4(Vector4 a_blk1, Vector4 a_blk2, Vector4 a_blk3, Vector4 a_blk4) : this(a_blk1.X, a_blk1.Y, a_blk1.Z, a_blk1.W,
                                                                                              a_blk2.X, a_blk2.Y, a_blk2.Z, a_blk2.W,
                                                                                              a_blk3.X, a_blk3.Y, a_blk3.Z, a_blk3.W,
                                                                                              a_blk4.X, a_blk4.Y, a_blk4.Z, a_blk4.W)
        {

        }
        /// <summary>
        /// Creates a new matrix with the specified values
        /// </summary>
        public Matrix4(float a_0_0, float a_0_1, float a_0_2, float a_0_3,
                       float a_1_0, float a_1_1, float a_1_2, float a_1_3,
                       float a_2_0, float a_2_1, float a_2_2, float a_2_3,
                       float a_3_0, float a_3_1, float a_3_2, float a_3_3)
        {
            m_data = new float[16];

            m_data[0] = a_0_0;
            m_data[1] = a_0_1;
            m_data[2] = a_0_2;
            m_data[3] = a_0_3;

            m_data[4] = a_1_0;
            m_data[5] = a_1_1;
            m_data[6] = a_1_2;
            m_data[7] = a_1_3;

            m_data[8] = a_2_0;
            m_data[9] = a_2_1;
            m_data[10] = a_2_2;
            m_data[11] = a_2_3;

            m_data[12] = a_3_0;
            m_data[13] = a_3_1;
            m_data[14] = a_3_2;
            m_data[15] = a_3_3;
        }
        /// <summary>
        /// Copies the values from another matrix
        /// </summary>
        public Matrix4(Matrix4 a_other)
        {
            m_data = new float[16];

            for (int i = 0; i < 16; ++i)
            {
                m_data[i] = a_other.m_data[i];
            }
        }

        public override string ToString()
        {
            return $"( ({m_data[0]}, {m_data[1]}, {m_data[2]}, {m_data[3]}), ({m_data[4]}, {m_data[5]}, {m_data[6]}, {m_data[7]}), ({m_data[8]}, {m_data[9]}, {m_data[10]}, {m_data[11]}), ({m_data[12]}, {m_data[13]}, {m_data[14]}, {m_data[15]}) )";
        }

        /// <summary>
        /// Converts the matrix to an array
        /// </summary>
        public float[] ToArray()
        {
            return m_data;
        }

        /// <summary>
        /// Generates an inverse of the matrix
        /// </summary>
        /// <param name="a_mat">Matrix to generate inverse of</param>
        /// <returns>Inverse of the matrix</returns>
        public static Matrix4 Inverse(Matrix4 a_mat)
        {
            // I invoke the I see 2 pages of greek therefore fuck that C+V
            // Do not feel like translating the dead sea scrolls again when it is "just" an inverse
            // That and people can probably implement it better then me
            float c00 = a_mat[2][2] * a_mat[3][3] - a_mat[3][2] * a_mat[2][3];
            float c02 = a_mat[1][2] * a_mat[3][3] - a_mat[3][2] * a_mat[1][3];
            float c03 = a_mat[1][2] * a_mat[2][3] - a_mat[2][2] * a_mat[1][3];

            float c04 = a_mat[2][1] * a_mat[3][3] - a_mat[3][1] * a_mat[2][3];
            float c06 = a_mat[1][1] * a_mat[3][3] - a_mat[3][1] * a_mat[1][3];
            float c07 = a_mat[1][1] * a_mat[2][3] - a_mat[2][1] * a_mat[1][3];

            float c08 = a_mat[2][1] * a_mat[3][2] - a_mat[3][1] * a_mat[2][2];
            float c10 = a_mat[1][1] * a_mat[3][2] - a_mat[3][1] * a_mat[1][2];
            float c11 = a_mat[1][1] * a_mat[2][2] - a_mat[2][1] * a_mat[1][2];

            float c12 = a_mat[2][0] * a_mat[3][3] - a_mat[3][0] * a_mat[2][3];
            float c14 = a_mat[1][0] * a_mat[3][3] - a_mat[3][0] * a_mat[1][3];
            float c15 = a_mat[1][0] * a_mat[2][3] - a_mat[2][0] * a_mat[1][3];

            float c16 = a_mat[2][0] * a_mat[3][2] - a_mat[3][0] * a_mat[2][2];
            float c18 = a_mat[1][0] * a_mat[3][2] - a_mat[3][0] * a_mat[1][2];
            float c19 = a_mat[1][0] * a_mat[2][2] - a_mat[2][0] * a_mat[1][2];

            float c20 = a_mat[2][0] * a_mat[3][1] - a_mat[3][0] * a_mat[2][1];
            float c22 = a_mat[1][0] * a_mat[3][1] - a_mat[3][0] * a_mat[1][1];
            float c23 = a_mat[1][0] * a_mat[2][1] - a_mat[2][0] * a_mat[1][1];

            Vector4 fac0 = new Vector4(c00, c00, c02, c03);
            Vector4 fac1 = new Vector4(c04, c04, c06, c07);
            Vector4 fac2 = new Vector4(c08, c08, c10, c11);
            Vector4 fac3 = new Vector4(c12, c12, c14, c15);
            Vector4 fac4 = new Vector4(c16, c16, c18, c19);
            Vector4 fac5 = new Vector4(c20, c20, c22, c23);

            Vector4 vec0 = new Vector4(a_mat[1][0], a_mat[0][0], a_mat[0][0], a_mat[0][0]);
            Vector4 vec1 = new Vector4(a_mat[1][1], a_mat[0][1], a_mat[0][1], a_mat[0][1]);
            Vector4 vec2 = new Vector4(a_mat[1][2], a_mat[0][2], a_mat[0][2], a_mat[0][2]);
            Vector4 vec3 = new Vector4(a_mat[1][3], a_mat[0][3], a_mat[0][3], a_mat[0][3]);

            Vector4 inv0 = new Vector4(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            Vector4 inv1 = new Vector4(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            Vector4 inv2 = new Vector4(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            Vector4 inv3 = new Vector4(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            Vector4 signA = new Vector4(+1, -1, +1, -1);
            Vector4 signB = new Vector4(-1, +1, -1, +1);

            Matrix4 inverse = new Matrix4(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

            Vector4 row0 = new Vector4(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

            Vector4 dot0 = new Vector4(a_mat[0] * row0);
            float dot1 = (dot0.X + dot0.Y) + (dot0.Z + dot0.W);

            float oneOverDeterminant = 1.0f / dot1;

            return inverse * new Matrix4(oneOverDeterminant);
        }
        /// <summary>
        /// Generates a transformation matrix from the specified values
        /// </summary>
        /// <param name="a_translation">Translation to use</param>
        /// <param name="a_rotation">Rotation to use</param>
        /// <param name="a_scale">Scale to use</param>
        /// <returns>Transformation matrix</returns>
        public static Matrix4 FromTransform(Vector3 a_translation, Quaternion a_rotation, Vector3 a_scale)
        {
            Matrix4 translation = new Matrix4
            (
                new Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                new Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                new Vector4(0.0f, 0.0f, 1.0f, 0.0f),
                new Vector4(a_translation, 1.0f)
            );
            Matrix4 scale = new Matrix4
            (
                a_scale.X, 0.0f, 0.0f, 0.0f,
                0.0f, a_scale.Y, 0.0f, 0.0f,
                0.0f, 0.0f, a_scale.Z, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );

            return scale * a_rotation.ToMatrix() * translation;
        }

        /// <summary>
        /// Generates a transform matrix looking to the specified direction
        /// </summary>
        /// <param name="a_pos">Position to look from</param>
        /// <param name="a_forward">Direction to look at</param>
        /// <param name="a_up">Up vector</param>
        /// <returns>Transform matrix</returns>
        public static Matrix4 LookTo(Vector3 a_pos, Vector3 a_forward, Vector3 a_up)
        {
            Vector3 zAxis = Vector3.Normalized(a_forward);
            Vector3 xAxis = Vector3.Normalized(Vector3.Cross(a_up, zAxis));
            Vector3 yAxis = Vector3.Cross(zAxis, xAxis);

            return new Matrix4
            (
                new Vector4(xAxis, 0.0f),
                new Vector4(yAxis, 0.0f),
                new Vector4(zAxis, 0.0f),
                new Vector4(a_pos, 1.0f)
            );
        }
        /// <summary>
        /// Generates a transform matrix looking at the specified target
        /// </summary>
        /// <param name="a_pos">Position to look from</param>
        /// <param name="a_target">Target to look at</param>
        /// <param name="a_up">Up vector</param>
        /// <returns>Transform matrix</returns>
        public static Matrix4 LookAt(Vector3 a_pos, Vector3 a_target, Vector3 a_up)
        {
            return LookTo(a_pos, a_target - a_pos, a_up);
        }

        /// <summary>
        /// Generates a orthographic perspective matrix
        /// </summary>
        /// <param name="a_x">Width of the view</param>
        /// <param name="a_y">Height of the view</param>
        /// <param name="a_near">Near plane</param>
        /// <param name="a_far">Far plane</param>
        /// <returns>Orthographic perspective matrix</returns>
        public static Matrix4 CreateOrthographic(float a_x, float a_y, float a_near, float a_far)
        {
            return new Matrix4
            (
                2.0f / a_x, 0.0f,       0.0f,                       0.0f,
                0.0f,       2.0f / a_y, 0.0f,                       0.0f,
                0.0f,       0.0f,       1.0f / (a_far - a_near),    0.0f,
                0.0f,       0.0f,       -a_near / (a_far - a_near), 1.0f
            );
        }

        /// <summary>
        /// Decomposes a transform matrix into its components
        /// </summary>
        /// <param name="a_mat">Matrix to decompose</param>
        /// <param name="a_translation">Translation component</param>
        /// <param name="a_rotation">Rotation component</param>
        /// <param name="a_scale">Scale component</param>
        public static void Decompose(Matrix4 a_mat, out Vector3 a_translation, out Quaternion a_rotation, out Vector3 a_scale)
        {
            a_translation = a_mat[3].XYZ;

            Vector3 scalarX = a_mat[0].XYZ;
            Vector3 scalarY = a_mat[1].XYZ;
            Vector3 scalarZ = a_mat[2].XYZ;

            a_scale = new Vector3(scalarX.Magnitude, scalarY.Magnitude, scalarZ.Magnitude);

            a_rotation = Quaternion.FromDirectionVectors(scalarX / a_scale.X, scalarY / a_scale.Y, scalarZ / a_scale.Z);
        }
        /// <summary>
        /// Decomposes a transform matrix into its components
        /// </summary>
        /// <param name="a_mat">Matrix to decompose</param>
        /// <param name="a_translation">Translation component</param>
        /// <param name="a_right">Right component</param>
        /// <param name="a_up">Up component</param>
        /// <param name="a_forward">Forward component</param>
        /// <param name="a_scale">Scale component</param>
        public static void Decompose(Matrix4 a_mat, out Vector3 a_translation, out Vector3 a_right, out Vector3 a_up, out Vector3 a_forward, out Vector3 a_scale)
        {
            a_translation = a_mat[3].XYZ;

            Vector3 scalarX = a_mat[0].XYZ;
            Vector3 scalarY = a_mat[1].XYZ;
            Vector3 scalarZ = a_mat[2].XYZ;

            a_scale = new Vector3(scalarX.Magnitude, scalarY.Magnitude, scalarZ.Magnitude);

            a_right = scalarX / a_scale.X;
            a_up = scalarY / a_scale.Y;
            a_forward = scalarZ / a_scale.Z;
        }

        /// <summary>
        /// Decomposes the matrix into its components
        /// </summary>
        /// <param name="a_translation">Translation component</param>
        /// <param name="a_rotation">Rotation component</param>
        /// <param name="a_scale">Scale component</param>
        public void Decompose(out Vector3 a_translation, out Quaternion a_rotation, out Vector3 a_scale)
        {
            Decompose(this, out a_translation, out a_rotation, out a_scale);
        }

        /// <summary>
        /// Transposes the matrix
        /// </summary>
        /// <param name="a_matrix">Matrix to transpose</param>
        /// <returns>Transposed matrix</returns>
        public static Matrix4 Transpose(Matrix4 a_matrix)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    mat.m_data[i * 4 + j] = a_matrix.m_data[i + j * 4];
                }
            }

            return mat;
        }

        public static Matrix4 operator *(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            // Not the fastest matrix multiplication but should work for now
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    for (int k = 0; k < 4; ++k)
                    {
                        mat.m_data[i * 4 + j] += a_lhs.m_data[i * 4 + k] * a_rhs.m_data[k * 4 + j]; 
                    }
                }
            }

            return mat;
        }
        public static Matrix4 operator +(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 16; ++i)
            {
                mat.m_data[i] = a_lhs.m_data[i] + a_rhs.m_data[i];
            }

            return mat;
        }
        public static Matrix4 operator -(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            for (int i = 0; i < 16; ++i)
            {
                mat.m_data[i] = a_lhs.m_data[i] + a_rhs.m_data[i];
            }

            return mat;
        }

        public static Vector4 operator *(Matrix4 a_lhs, Vector4 a_rhs)
        {
            Vector4 vec = new Vector4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    vec[i] += a_lhs.m_data[i * 4 + j] * a_rhs[j];
                }
            }

            return vec;
        }
        public static Vector4 operator *(Vector4 a_lhs, Matrix4 a_rhs)
        {
            Vector4 vec = new Vector4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    vec[i] += a_lhs[j] * a_rhs.m_data[j * 4 + i];
                }
            }

            return vec;
        }
    }
}