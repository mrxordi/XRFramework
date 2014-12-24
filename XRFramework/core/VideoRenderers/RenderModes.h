#pragma once

//Thats all for now, i'm not expecting to implement more before leaving beta stage

enum ERenderFormat {
	RENDER_FMT_NONE = 0,
	RENDER_FMT_YUV420P,
	RENDER_FMT_YUV420P10,
	RENDER_FMT_YUV420P16,
	RENDER_FMT_NV12,
	RENDER_FMT_UYVY422,
	RENDER_FMT_YUYV422,
	/*RENDER_FMT_DXVA,
	RENDER_FMT_OMXEGL,
	RENDER_FMT_CVBREF,
	RENDER_FMT_BYPASS,
	RENDER_FMT_EGLIMG,
	RENDER_FMT_MEDIACODEC,
	RENDER_FMT_IMXMAP,
	RENDER_FMT_MMAL,
*/
};

// Render Methods
enum RenderMethods
{
	RENDER_METHOD_AUTO = 0,
	RENDER_METHOD_D3D_PS,
	RENDER_METHOD_SW,
};