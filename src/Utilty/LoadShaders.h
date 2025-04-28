//////////////////////////////////////////////////////////////////////////////
//
//  --- LoadShaders.h ---
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOAD_SHADERS_H__
#define __LOAD_SHADERS_H__

#include <iostream>
#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

    //----------------------------------------------------------------------------
    // ShaderInfo 結構，用於儲存 shader 的類型、檔案名和對應的 shader 物件
    typedef struct {
        GLenum       type;
        const char* filename;
        GLuint       shader;
    } ShaderInfo;

    // ReadShader 函數的聲明
    static const GLchar* ReadShader(const char* filename);

    // LoadShaders 函數的聲明
    GLuint LoadShaders(ShaderInfo* shaders);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif // __LOAD_SHADERS_H__
