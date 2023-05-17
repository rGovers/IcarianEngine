namespace IcarianEngine.Maths
{
    public struct Matrix4
    {
        public static readonly Matrix4 Identity = new Matrix4(1.0f);

        float[] m_data;

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

        public Matrix4(float a_val) : this(a_val, 0.0f, 0.0f, 0.0f,
                                           0.0f, a_val, 0.0f, 0.0f,
                                           0.0f, 0.0f, a_val, 0.0f,
                                           0.0f, 0.0f, 0.0f, a_val)
        {
        }
        public Matrix4(Vector4 a_blk1, Vector4 a_blk2, Vector4 a_blk3, Vector4 a_blk4) : this(a_blk1.X, a_blk1.Y, a_blk1.Z, a_blk1.W,
                                                                                              a_blk2.X, a_blk2.Y, a_blk2.Z, a_blk2.W,
                                                                                              a_blk3.X, a_blk3.Y, a_blk3.Z, a_blk3.W,
                                                                                              a_blk4.X, a_blk4.Y, a_blk4.Z, a_blk4.W)
        {

        }
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
        public Matrix4(Matrix4 a_other)
        {
            m_data = new float[16];

            for (int i = 0; i < 16; ++i)
            {
                m_data[i] = a_other.m_data[i];
            }
        }

        public float[] ToArray()
        {
            return m_data;
        }

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
        public static void Decompose(Matrix4 a_mat, out Vector3 a_translation, out Quaternion a_rotation, out Vector3 a_scale)
        {
            a_translation = a_mat[3].XYZ;

            Vector3 scalarX = a_mat[0].XYZ;
            Vector3 scalarY = a_mat[1].XYZ;
            Vector3 scalarZ = a_mat[2].XYZ;

            a_scale = new Vector3(scalarX.Magnitude, scalarY.Magnitude, scalarZ.Magnitude);

            a_rotation = Quaternion.FromDirectionVectors(scalarX / a_scale.X, scalarY / a_scale.Y, scalarZ / a_scale.Z);
        }
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

        public void Decompose(out Vector3 a_translation, out Quaternion a_rotation, out Vector3 a_scale)
        {
            Decompose(this, out a_translation, out a_rotation, out a_scale);
        }

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
    }
}