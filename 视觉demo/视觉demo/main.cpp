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
GLFrame objectFrame;

#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

void SetupRC() {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    shaderManager.InitializeStockShaders();
    
//    objectFrame.MoveForward(5);
    
    glEnable(GL_DEPTH_TEST);
    
    //地板
    floorBatch.Begin(GL_LINES, 324);
    for (GLfloat x = -20.0; x <= 20.0; x+=0.5) {
        floorBatch.Vertex3f(x, -0.55, 20.0);
        floorBatch.Vertex3f(x, -0.55, -20.0);
        floorBatch.Vertex3f(20.0, -0.55, x);
        floorBatch.Vertex3f(-20.0, -0.55, x);
    }
    floorBatch.End();
    
    //大球
    gltMakeSphere(torusBatch, 0.4, 40, 80);
    
    //小球
    gltMakeSphere(sphereBatch, 0.1f, 26, 13);
    
    //随机小球位置 NUM_SPHERES个
    for (int i = 0; i < NUM_SPHERES; i++) {
        GLfloat x = (GLfloat)((rand() % 400) - 200) * 0.1;
        GLfloat z = (GLfloat)((rand() % 400) - 200) * 0.1;
        
        spheres[i].SetOrigin(x, 0.0f, z);
    }
}

//开始渲染
void RenderScene(void) {
    // 1
    //地板颜色
    static GLfloat vFloorColor[] = {0.0, 1.0, 0.0, 1.0};
    //打球颜色
    static GLfloat vTorusColor[] = {1, 0, 0, 1};
    //小球颜色
    static GLfloat vSphereColor[] = {0, 0, 1, 1};
    
    //2
    //此处需要使用静态变量 原因CStopWatch中有一个成员变量m_LastCount，需要记录，当调用GetElapsedSeconds时，内部实现是当前的秒数与m_LastCount中的秒数进行插值
    static CStopWatch rotTimer;
    //旋转度数
    float yRot = rotTimer.GetElapsedSeconds() * 60;
    
    //3
    //清除一个或一组特定的缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    //4.观察者
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    //入栈-保存观察者位置
    modelViewMatrix.PushMatrix(mCamera);
    
//    modelViewMatrix.PushMatrix(objectFrame);
    
    //5渲染地板
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor);
    floorBatch.Draw();
    
    //光源
    M3DVector4f vLightPos = {0.0f, 10.0f, 5.0f, 1.0f};
    
    //6渲染大球
    modelViewMatrix.Translate(0.0f, 0.0f, -3.0f);
    modelViewMatrix.PushMatrix();
    modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPos, vTorusColor);
    torusBatch.Draw();
    modelViewMatrix.PopMatrix();
    
    //7渲染50个（NUM_SPHERES）随机小球
    for (int i = 0; i < NUM_SPHERES; i++) {
        modelViewMatrix.PushMatrix();
        modelViewMatrix.MultMatrix(spheres[i]);
        shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPos, vSphereColor);
        sphereBatch.Draw();
        modelViewMatrix.PopMatrix();
    }
    
    //8渲染一个小球绕着打球旋转
    modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
    modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);
//    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vSphereColor);

    shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightPos, vSphereColor);
    sphereBatch.Draw();
    
    //出栈-与上面观察者的入栈 保持出入栈一致
    modelViewMatrix.PopMatrix();
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
    //9触发重新绘制
    glutPostRedisplay();
}

//窗口大小改变时接受新的宽度和高度，其中0，0代表窗口中视图的左下角坐标，w，h代表像素
void ChangeSize(int width, int height)
{
    glViewport(0, 0, width, height);
    
    viewFrustum.SetPerspective(35.0f, float(width)/float(height), 1.0f, 100.0f);
    
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

void SpecialKeys(int key, int x, int y) {
    float liner = 0.1f;
    float angular = float(m3dDegToRad(5));
    
    if (key == GLUT_KEY_UP) {
        cameraFrame.MoveForward(liner);
//        objectFrame.MoveForward(liner);
    }
    
    if (key == GLUT_KEY_DOWN) {
        cameraFrame.MoveForward(-liner);
//        objectFrame.MoveForward(-liner);
    }
    
    if (key == GLUT_KEY_LEFT) {
        cameraFrame.RotateWorld(angular, 0, 1, 0);
//        objectFrame.RotateWorld(angular, 0, 1, 0);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, 0, 1, 0);
//        objectFrame.RotateWorld(-angular, 0, 1, 0);
    }
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
    glutSpecialFunc(SpecialKeys);

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
