//
//  transformation.cpp
//  glcodes
//
//  Created by chenbingfeng on 15/4/26.
//  Copyright (c) 2015年 chenbingfeng. All rights reserved.
//

#include "transformation.h"


#include <math.h>
#include <unistd.h>

#include <iostream>
#include <chrono>

#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>

#include "utils.h"
#include "SOIL.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

static GLfloat vertices[] = {
    -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.3f, 1.0f, 1.0f,
    -0.5f, -0.5f, 0.8f, 0.0f, 1.0f
};

static GLuint elements[] = {
    0,1,2,
    2,3,0
};

static const char* vertexSource =
"#version 150\n"
"\n"
"in vec2 position;\n"
"in float color;\n"
"in vec2 texcoord;\n"
"uniform mat4 trans;\n"
"uniform mat4 view;\n"
"uniform mat4 proj;\n"
"out vec2 Texcoord;\n"
"out vec3 Color;\n"
"\n"
"\n"
"void main()\n"
"{\n"
"    Texcoord = texcoord * vec2(1.0, -1.0);\n"
"    Color = vec3(color);\n"
"    gl_Position = proj * view * trans * vec4(position.x, -position.y, 0.0, 1.0);\n"
"}\n";

static const char* fragmentSource =
"#version 150\n"
"\n"
"in vec3 Color;\n"
"in vec2 Texcoord;\n"
"uniform vec3 triangleColor;\n"
"\n"
"out vec4 outColor;\n"
"uniform sampler2D texCat;\n"
"uniform sampler2D texDog;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 c1 = texture(texCat, Texcoord);\n"
"    vec4 c2 = texture(texDog, Texcoord);\n"
"    outColor = mix(c1, c2, 0.5);\n"
"}\n";
static char clog[2014];
static GLuint vbo;
static GLuint vao;
static GLuint ebo;
static GLint uniColor;
static GLuint texCat;
static GLuint texDog;
static decltype(std::chrono::high_resolution_clock::now()) t_start;
static glm::mat4 trans;
static GLuint uniTrans;
static GLuint uniView;
static GLuint uniProj;
static void init()
{
    LOG("GL_RENDERER", glGetString(GL_RENDERER),  " GL_VERSION = ",glGetString(GL_VERSION));
    
    //vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    //vbo
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    
    //shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        LOG("vertex shader compile failed.");
        glGetShaderInfoLog(vertexShader, 1024, NULL, clog);
        LOG(clog);
        exit(1);
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        LOG("fragment shader compile failed.");
        glGetShaderInfoLog(fragmentShader, 1024, NULL, clog);
        LOG(clog);
        exit(1);
    }
    
    //as program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);
    
    //attributes
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 1, GL_FLOAT, GL_FALSE, 5*sizeof(float), (GLvoid*)(2*sizeof(float)));
    
    uniColor = glGetUniformLocation(shaderProgram, "triangleColor");
    glUniform3f(uniColor, 1.0f, 0.0f, 0.0f);
    
    glGenTextures(1, &texCat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texCat);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    int width, height;
    unsigned char* image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    
    glGenTextures(1, &texDog);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texDog);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    
    GLuint uniTexCat = glGetUniformLocation(shaderProgram, "texCat");
    glUniform1i(uniTexCat, 0);
    GLuint uniTexDog = glGetUniformLocation(shaderProgram, "texDog");
    glUniform1i(uniTexDog, 1);
    
    
    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (GLvoid*)(3*sizeof(float)));
    //    glGenTextures(1, &textureId);
    //    glBindTexture(GL_TEXTURE_2D, textureId);
    t_start = std::chrono::high_resolution_clock::now();
    
    //view matrix
    glm::mat4 view = glm::lookAt(glm::vec3(1.2f,1.2f,1.2f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
    
    //projection matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 1.0f, 10.0f);
    uniTrans = glGetUniformLocation(shaderProgram, "trans");
    uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
    uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    

}

static void display()
{
    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
    glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    trans = glm::rotate(trans, glm::radians(1.0f), glm::vec3(0.0f,0.0f,1.0f));//以某个方向，旋转某个角度
    glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
    //    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glutSwapBuffers();
    sleep(0.33);
    glutPostRedisplay();
}

static void reshape(int w, int h)
{
    
}

void transformation(){
    LOG("gl Test Code");
    int argc = 0;
    char * argv[] = {0};
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE |GLUT_3_2_CORE_PROFILE);
    glutInitWindowPosition(100,100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("lol model view");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    init();
    glutMainLoop();
}