#define MATRIX_TABLE(F, D) \
    F(0, M00, D) \
    F(1, M01, D) \
    F(2, M02, D) \
    F(3, M03, D) \
    F(4, M10, D) \
    F(5, M11, D) \
    F(6, M12, D) \
    F(7, M13, D) \
    F(8, M20, D) \
    F(9, M21, D) \
    F(10, M22, D) \
    F(11, M23, D) \
    F(12, M30, D) \
    F(13, M31, D) \
    F(14, M32, D) \
    F(15, M33, D) 

#define MATRIX_VAR(I, V, D) public float V;
#define MATRIX_ITER(I, V, D) D[I] = V;
#define MATRIX_MITER(I, V, D) V = D[I];
#define MATRIX_SET(I, V, D) V = D.V;
#define MATRIX_ADD(I, V, D) D.V = a_lhs.V + a_rhs.V;
#define MATRIX_SUB(I, V, D) D.V = a_lhs.V - a_rhs.V;

namespace IcarianEngine.Maths
{
    public struct Matrix4
    {
        /// <summary>
        /// Identity Matrix
        /// </summary>
        public static readonly Matrix4 Identity = new Matrix4(1.0f);

        MATRIX_TABLE(MATRIX_VAR, 0)

        /// <summary>
        /// Returns the row at the specified index
        /// </summary>
        public Vector4 this[int a_key]
        {
            get
            {
                switch (a_key)
                {
                case 0:
                {
                    return new Vector4(M00, M01, M02, M03);
                }
                case 1:
                {
                    return new Vector4(M10, M11, M12, M13);
                }
                case 2:
                {
                    return new Vector4(M20, M21, M22, M23);
                }
                case 3:
                {
                    return new Vector4(M30, M31, M32, M33);
                }
                }

                return new Vector4(float.NaN);
            }
            set
            {
                switch (a_key)
                {
                case 0:
                {
                    M00 = value.X;
                    M01 = value.Y;
                    M02 = value.Z;
                    M03 = value.W;

                    break;
                }
                case 1:
                {
                    M10 = value.X;
                    M11 = value.Y;
                    M12 = value.Z;
                    M13 = value.W;

                    break;
                }
                case 2:
                {
                    M20 = value.X;
                    M21 = value.Y;
                    M22 = value.Z;
                    M23 = value.W;

                    break;
                }
                case 3:
                {
                    M30 = value.X;
                    M31 = value.Y;
                    M32 = value.Z;
                    M33 = value.W;

                    break;
                }
                }
            }
        }

        public Matrix4(float a_val) : this(a_val, 0.0f, 0.0f, 0.0f,
                                           0.0f, a_val, 0.0f, 0.0f,
                                           0.0f, 0.0f, a_val, 0.0f,
                                           0.0f, 0.0f, 0.0f, a_val)
        {

        }

        public Matrix4(float[] a_mat)
        {
            MATRIX_TABLE(MATRIX_MITER, a_mat);
        }

        public Matrix4(Vector4 a_blk1, Vector4 a_blk2, Vector4 a_blk3, Vector4 a_blk4) : this(a_blk1.X, a_blk1.Y, a_blk1.Z, a_blk1.W,
                                                                                              a_blk2.X, a_blk2.Y, a_blk2.Z, a_blk2.W,
                                                                                              a_blk3.X, a_blk3.Y, a_blk3.Z, a_blk3.W,
                                                                                              a_blk4.X, a_blk4.Y, a_blk4.Z, a_blk4.W)
        {

        }

        public Matrix4(float a_00, float a_01, float a_02, float a_03,
                       float a_10, float a_11, float a_12, float a_13,
                       float a_20, float a_21, float a_22, float a_23,
                       float a_30, float a_31, float a_32, float a_33)
        {
            M00 = a_00;
            M01 = a_01;
            M02 = a_02;
            M03 = a_03;

            M10 = a_10;
            M11 = a_11;
            M12 = a_12;
            M13 = a_13;

            M20 = a_20;
            M21 = a_21;
            M22 = a_22;
            M23 = a_23;

            M30 = a_30;
            M31 = a_31;
            M32 = a_32;
            M33 = a_33;
        }

        public Matrix4(Matrix4 a_other)
        {
            MATRIX_TABLE(MATRIX_SET, a_other);
        }

        public override string ToString()
        {
            return $"( ({M00}, {M01}, {M02}, {M03}), ({M10}, {M11}, {M12}, {M13}), ({M20}, {M21}, {M22}, {M23}), ({M30}, {M31}, {M32}, {M33}) )";
        }

        /// <summary>
        /// Converts the Matrix to an array
        /// </summary>
        /// <returns>The Matrix as an array</returns>
        public float[] ToArray()
        {
            float[] data = new float[16];

            MATRIX_TABLE(MATRIX_ITER, data);

            return data;
        }

        /// <summary>
        /// Generates an inverse of the Matrix
        /// </summary>
        /// <param name="a_mat">Matrix to generate inverse of</param>
        /// <returns>Inverse of the matrix</returns>
        public static Matrix4 Inverse(Matrix4 a_mat)
        {
            // I invoke the I see 2 pages of greek therefore fuck that C+V
            // Do not feel like translating the dead sea scrolls again when it is "just" an inverse
            // That and people can probably implement it better then me
            float c00 = a_mat.M22 * a_mat.M33 - a_mat.M32 * a_mat.M23;
            float c02 = a_mat.M12 * a_mat.M33 - a_mat.M32 * a_mat.M13;
            float c03 = a_mat.M12 * a_mat.M23 - a_mat.M22 * a_mat.M13;

            float c04 = a_mat.M21 * a_mat.M33 - a_mat.M31 * a_mat.M23;
            float c06 = a_mat.M11 * a_mat.M33 - a_mat.M31 * a_mat.M13;
            float c07 = a_mat.M11 * a_mat.M23 - a_mat.M21 * a_mat.M13;

            float c08 = a_mat.M21 * a_mat.M32 - a_mat.M31 * a_mat.M22;
            float c10 = a_mat.M11 * a_mat.M32 - a_mat.M31 * a_mat.M12;
            float c11 = a_mat.M11 * a_mat.M22 - a_mat.M21 * a_mat.M12;

            float c12 = a_mat.M20 * a_mat.M33 - a_mat.M30 * a_mat.M23;
            float c14 = a_mat.M10 * a_mat.M33 - a_mat.M30 * a_mat.M13;
            float c15 = a_mat.M10 * a_mat.M23 - a_mat.M20 * a_mat.M13;

            float c16 = a_mat.M20 * a_mat.M32 - a_mat.M30 * a_mat.M22;
            float c18 = a_mat.M10 * a_mat.M32 - a_mat.M30 * a_mat.M12;
            float c19 = a_mat.M10 * a_mat.M22 - a_mat.M20 * a_mat.M12;

            float c20 = a_mat.M20 * a_mat.M31 - a_mat.M30 * a_mat.M21;
            float c22 = a_mat.M10 * a_mat.M31 - a_mat.M30 * a_mat.M11;
            float c23 = a_mat.M10 * a_mat.M21 - a_mat.M20 * a_mat.M11;

            Vector4 fac0 = new Vector4(c00, c00, c02, c03);
            Vector4 fac1 = new Vector4(c04, c04, c06, c07);
            Vector4 fac2 = new Vector4(c08, c08, c10, c11);
            Vector4 fac3 = new Vector4(c12, c12, c14, c15);
            Vector4 fac4 = new Vector4(c16, c16, c18, c19);
            Vector4 fac5 = new Vector4(c20, c20, c22, c23);

            Vector4 vec0 = new Vector4(a_mat.M10, a_mat.M00, a_mat.M00, a_mat.M00);
            Vector4 vec1 = new Vector4(a_mat.M11, a_mat.M01, a_mat.M01, a_mat.M01);
            Vector4 vec2 = new Vector4(a_mat.M12, a_mat.M02, a_mat.M02, a_mat.M02);
            Vector4 vec3 = new Vector4(a_mat.M13, a_mat.M03, a_mat.M03, a_mat.M03);

            Vector4 inv0 = new Vector4(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            Vector4 inv1 = new Vector4(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            Vector4 inv2 = new Vector4(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            Vector4 inv3 = new Vector4(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            Vector4 signA = new Vector4(+1, -1, +1, -1);
            Vector4 signB = new Vector4(-1, +1, -1, +1);

            Matrix4 inverse = new Matrix4(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

            Vector4 row0 = new Vector4(inverse.M00, inverse.M10, inverse.M20, inverse.M30);

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
        /// Generates a transform Matrix looking to the specified direction
        /// </summary>
        /// <param name="a_pos">Position to look from</param>
        /// <param name="a_forward">Direction to look at</param>
        /// <param name="a_up">Up vector</param>
        /// <returns>Transform Matrix</returns>
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
        /// Generates a transform Matrix looking at the specified target
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
        /// Generates a orthographic protection Matrix
        /// </summary>
        /// <param name="a_x">Width of the view</param>
        /// <param name="a_y">Height of the view</param>
        /// <param name="a_near">Near plane</param>
        /// <param name="a_far">Far plane</param>
        /// <returns>Orthographic protection matrix</returns>
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
        /// Generates a perspective Matrix
        /// </summary>
        /// <param name="a_fov">Field of view in radians.</param>
        /// <param name="a_aspect">Aspect ratio</param>
        /// <param name="a_near">Near plane</param>
        /// <param name="a_far">Far plane</param>
        /// <returns>Perspective Matrix</returns>
        public static Matrix4 CreatePerspective(float a_fov, float a_aspect, float a_near, float a_far)
        {
            float halfFov = a_fov * 0.5f;
            float f = Mathf.Cos(halfFov) / Mathf.Sin(halfFov);

            // Apparently no projection matrix is correct and had to eyeball until the view projection matrix looked right
            // No idea if this is correct but looks right
            return new Matrix4
            (
                a_aspect * f, 0.0f,  0.0f,                                 0.0f,
                0.0f,         f,     0.0f,                                 0.0f,
                0.0f,         0.0f,  a_far / (a_near - a_far),             -1.0f,
                0.0f,         0.0f,  -(a_far * a_near) / (a_far - a_near), 0.0f
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
        /// Decomposes the Matrix into its base components
        /// </summary>
        /// <param name="a_translation">Translation component</param>
        /// <param name="a_rotation">Rotation component</param>
        /// <param name="a_scale">Scale component</param>
        public void Decompose(out Vector3 a_translation, out Quaternion a_rotation, out Vector3 a_scale)
        {
            Decompose(this, out a_translation, out a_rotation, out a_scale);
        }

        /// <summary>
        /// Transposes the Matrix
        /// </summary>
        /// <param name="a_matrix">Matrix to transpose</param>
        /// <returns>Transposed Matrix</returns>
        public static Matrix4 Transpose(Matrix4 a_matrix)
        {
            float[] mat = new float[16];

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    mat[i * 4 + j] = a_matrix[j][i];
                }
            }

            return new Matrix4(mat);
        }

        public static Matrix4 operator *(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            // Not the fastest matrix multiplication but should work for now
            float[] mat = new float[16];

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    for (int k = 0; k < 4; ++k)
                    {
                        mat[i * 4 + j] += a_lhs[i][k] * a_rhs[k][j];
                    }
                }
            }

            return new Matrix4(mat);
        }
        public static Matrix4 operator +(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            MATRIX_TABLE(MATRIX_ADD, mat);

            return mat;
        }
        public static Matrix4 operator -(Matrix4 a_lhs, Matrix4 a_rhs)
        {
            Matrix4 mat = new Matrix4(0.0f);

            MATRIX_TABLE(MATRIX_SUB, mat);

            return mat;
        }

        public static Vector4 operator *(Matrix4 a_lhs, Vector4 a_rhs)
        {
            Vector4 vec = new Vector4(0.0f);

            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    vec[i] += a_lhs[i][j] * a_rhs[j];
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
                    vec[i] += a_lhs[j] * a_rhs[j][i];
                }
            }

            return vec;
        }
    }
}