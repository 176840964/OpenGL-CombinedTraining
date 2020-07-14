//
//  main.cpp
//  视觉demo
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif
#include "GLTools.h"
#include <glut/glut.h>

GLShaderManager shaderManager;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLFrustum viewFrustum;
GLGeometryTransform transformPipeline;

GLTriangleBatch torusBatch;
GLTriangleBatch sphereBatch;
GLBatch floorBatch;

GLFrame cameraFrame;

#define NUM_SPHERE 50
GLFrame spheres[NUM_SPHERE];

//窗口大小改变时接受新的宽度和高度，其中0，0代表窗口中视图的左下角坐标，w，h代表像素
void ChangeSize(int width, int height)
{
    glViewport(0, 0, width, height);
    
    viewFrustum.SetPerspective(35.0f, float(width)/float(height), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

//为程序做一次性的设置
void SetupRC() {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    
    floorBatch.Begin(GL_LINES, 324);
    for (GLfloat x = -20.0; x <= 20.0; x+=0.5) {
        floorBatch.Vertex3f(x, -0.55, 20.0);
        floorBatch.Vertex3f(x, -0.55, -20.0);
        floorBatch.Vertex3f(20.0, -0.55, x);
        floorBatch.Vertex3f(-20.0, -0.55, x);
    }
    floorBatch.End();
    
    gltMakeSphere(torusBatch, 0.4, 40, 80);
}

//开始渲染
void RenderScene(void) {
    
    GLfloat vFloorColor[] = {0.0, 1.0, 0.0, 1.0};
    GLfloat vTorusColor[] = {1, 0, 0, 1};
    
    CStopWatch rotTimer;
    float yRot = rotTimer.GetElapsedSeconds() * 60;
    
    //清楚一个或一组特定的缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor);
    floorBatch.Draw();
    
    M3DVector4f vLightPos = {0, 10, 5, 1};
    
    modelViewMatrix.Translate(0, 0, -3);
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Rotate(yRot, 0, 1, 0);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPos, vTorusColor);
    torusBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
}

int main(int argc,char* argv[]) {

    //设置当前工作目录，针对MAC OS X
    gltSetWorkingDirectory(argv[0]);
    
    //初始化GLUT库
    glutInit(&argc, argv);
    /*初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
     双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区
     */
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    
    //GLUT窗口大小
    glutInitWindowSize(800,600);
    glutCreateWindow("OpenGL SphereWorld");
    
    //注册回调函数
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    //驱动程序的初始化中没有出现任何问题
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        return 1;
    }

    //调用SetupRC
    SetupRC();
    glutMainLoop();
    return 0;
}
