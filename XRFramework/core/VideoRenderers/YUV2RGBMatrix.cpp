#include "stdafxf.h"
#include "YUV2RGBMatrix.h"
#include "TransformMatrix.h"

//using namespace DirectX;
//
// Transformation matrixes for different colorspaces.
//
static float yuv_coef_bt601[4][4] = 
{
	{ 1.0f, 1.0f, 1.0f, 0.0f }, 
	{ 0.0f, -0.344f, 1.773f, 0.0f },
	{ 1.403f, -0.714f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f }
};

static float yuv_coef_bt709[4][4] = 
{
	{ 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, -0.1870f, 1.8556f, 0.0f },
	{ 1.5701f, -0.4664f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f }
};

static float yuv_coef_ebu[4][4] = 
{
	{ 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, -0.3960f, 2.029f, 0.0f },
	{ 1.140f, -0.581f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f }
};

static float yuv_coef_smtp240m[4][4] = 
{
	{ 1.0f, 1.0f, 1.0f, 0.0f },
	{ 0.0f, -0.2253f, 1.8270f, 0.0f },
	{ 1.5756f, -0.5000f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f }
};

static float** PickYUVConversionMatrix(unsigned flags) 
{
	// Pick the matrix.

	switch (CONF_FLAGS_YUVCOEF_MASK(flags)) 
	{
	case CONF_FLAGS_YUVCOEF_240M:
		return (float**)yuv_coef_smtp240m; break;
	case CONF_FLAGS_YUVCOEF_BT709:
		return (float**)yuv_coef_bt709; break;
	case CONF_FLAGS_YUVCOEF_BT601:
		return (float**)yuv_coef_bt601; break;
	case CONF_FLAGS_YUVCOEF_EBU:
		return (float**)yuv_coef_ebu; break;
	}

	return (float**)yuv_coef_bt601;
}

void CalculateYUVMatrix(TransformMatrix &matrix, unsigned int flags, ERenderFormat format, float black, float contrast) 
{
	TransformMatrix coef;

	matrix *= TransformMatrix::CreateScaler(contrast, contrast, contrast);
	matrix *= TransformMatrix::CreateTranslation(black, black, black);

	float(*conv)[4] = (float(*)[4])PickYUVConversionMatrix(flags);
	for (int row = 0; row < 3; row++)
		for (int col = 0; col < 4; col++)
			coef.m[row][col] = conv[col][row];
	coef.identity = false;

	matrix *= coef;

	matrix *= TransformMatrix::CreateTranslation(0.0, -0.5, -0.5);
	

	if (!(flags & CONF_FLAGS_YUV_FULLRANGE)) 
	{
		matrix *= TransformMatrix::CreateScaler(255.0f / (235 - 16),
			255.0f / (240 - 16), 
			255.0f / (240 - 16));
		matrix *= TransformMatrix::CreateTranslation(-16.0f / 255,
			-16.0f / 255, 
			-16.0f / 255);
	}

	if (format == RENDER_FMT_YUV420P10) 
	{
		matrix *= TransformMatrix::CreateScaler(65535.0f / 1023.0f,
			65535.0f / 1023.0f, 
			65535.0f / 1023.0f);
	}
}

YUV2RGBMatrix::YUV2RGBMatrix() 
{
	m_NeedRecalc = true;
	m_blacklevel = 0.0f;
	m_contrast = 0.0f;
	m_flags = 0;
	m_format = RENDER_FMT_NONE;
}

void YUV2RGBMatrix::SetParameters(float contrast, float blacklevel, unsigned int flags, ERenderFormat format) 
{
	if (m_contrast != contrast) 
	{
		m_NeedRecalc = true;
		m_contrast = contrast;
	}
	if (m_blacklevel != blacklevel) 
	{
		m_NeedRecalc = true;
		m_blacklevel = blacklevel;
	}
	if (m_flags != flags) 
	{
		m_NeedRecalc = true;
		m_flags = flags;
	}
	if (m_format != format) 
	{
		m_NeedRecalc = true;
		m_format = format;
	}
}

XMFLOAT4X4* YUV2RGBMatrix::Matrix() 
{
	if (m_NeedRecalc) 
	{
		TransformMatrix matrix;
		CalculateYUVMatrix(matrix, m_flags, m_format, m_blacklevel, m_contrast);

		m_mat = XMFLOAT4X4(matrix.m[0][0], matrix.m[1][0], matrix.m[2][0], 0.0f,
			matrix.m[0][1], matrix.m[1][1], matrix.m[2][1], 0.0f,
			matrix.m[0][2], matrix.m[1][2], matrix.m[2][2], 0.0f,
			matrix.m[0][3], matrix.m[1][3], matrix.m[2][3], 1.0f);

		m_NeedRecalc = false;
	}
	return &m_mat;
}