Transform feedback
==================
从流水线中拿一些数据，比如在几何着色器之后的顶点数目。

Up until now we've always sent vertex data to the graphics processor and only
produced drawn pixels in framebuffers in return. What if we want to retrieve the
vertices after they've passed through the vertex or geometry shaders? In this
chapter we'll look at a way to do this, known as *transform feedback*.

So far, we've used VBOs (Vertex Buffer Objects) to store vertices to be used for
drawing operations. The transform feedback extension allows shaders to write
vertices back to these as well. You could for example build a vertex shader that
simulates gravity and writes updated vertex positions back to the buffer. This
way you don't have to transfer this data back and forth from graphics memory to
main memory. On top of that, you get to benefit from the vast parallel
processing power of today's GPUs.直到现在，我们使用VBO来装将要被绘制的顶点数据。然而，变换反馈实际上能够允许着色器将一些顶点协会到VBO中。比如，你可以创建一个着色器来模拟重力，将顶点的位置更新回VBO。使用这个方法，这种顶点数据的变更就不再需要有数据在显卡和CPU之间传递了，增加了性能。

Basic feedback
==============

We'll start from scratch so that the final program will clearly demonstrate
how simple transform feedback is. Unfortunately there's no preview this time,
because we're not going to draw anything in this chapter! Although this feature
can be used to simplify effects like particle simulation, explaining these is a
bit beyond the scope of these articles. After you've understood the basics of
transform feedback, you'll be able to find and understand plenty of articles
around the web on these topics.我们从头做起，这样能够显示出变换反馈是多么简单。不过，因为我们不准备画任何东西出来，所以没有什么效果图了。虽然，这个特性能够简化诸如粒子模拟等特效，但是这已经超出了本文的讨论。当你理解了变换反馈之后，你就可以进一步理解网上很多相关话题的讨论了。

Let's start with a simple vertex shader.我们从一个简单的顶点着色器开始：

    const GLchar* vertexShaderSrc = GLSL(
        in float inValue;
        out float outValue;

        void main() {
            outValue = sqrt(inValue);
        }
    );

This vertex shader does not appear to make much sense. It doesn't set a
`gl_Position` and it only takes a single arbitrary float as input. Luckily, we
can use transform feedback to capture the result, as we'll see momentarily.这个顶点着色器好像没有多少意思。它没有设置gl_Position，而只是获取一个float作为输入。幸运地，我们可以使用变换反馈来获取它的结果，我们马上就能够看到：

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);

Compile the shader, create a program and attach the shader, but don't call
`glLinkProgram` yet! Before linking the program, we have to tell OpenGL which
output attributes we want to capture into a buffer.编译着色器，创建着色程序，绑定着色器到着色程序。但不要链接，在链接之前，我们需要告诉OpenGL我们需要抓住哪些输出attribute到buffer中。

    const GLchar* feedbackVaryings[] = { "outValue" };
    glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

The first parameter is self-explanatory, the second and third parameter specify
the length of the output names array and the array itself, and the final
parameter specifies how the data should be written.第一个参数是显然的，第二个参数指明了输出的名字数组的长度，第三个是名字数组，最后一个表示这些数据该怎么被写：

The following two formats are available:有如下2中方式可供选择：

- **GL_INTERLEAVED_ATTRIBS**: Write all attributes to a single buffer object.把所有的输出属性写到同一个缓冲对象中。
- **GL_SEPARATE_ATTRIBS**: Writes attributes to multiple buffer objects or at
different offsets into a buffer.把这些输出属性写到多个缓冲对象或者同一个缓冲对象的不同偏移位置。

Sometimes it is useful to have separate buffers for each attribute, but lets
keep it simple for this demo. Now that you've specified the output variables,
you can link and activate the program. That is because the linking process
depends on knowledge about the outputs.有时候，分开的写出十分有用。但在这里我们将保持尽量的简单。现在你已经告知了要输出的变量，现在可以链接和激活着色程序了。这种时间顺序是因为，链接阶段需要使用前面的输出capture映射关系。

    glLinkProgram(program);
    glUseProgram(program);

After that, create and bind the VAO:之后，我们创建和激活VAO:

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

Now, create a buffer with some input data for the vertex shader:并且创建一个VBO，并加入一些数据：

    GLfloat data[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

The numbers in `data` are the numbers we want the shader to calculate the square
root of and transform feedback will help us get the results back.在data的数字将被传到shader中，shader将对其开平方根，然后通过变换反馈来告诉我们结果。

With regards to vertex pointers, you know the drill by now:配置好输入attribute的配置：

    GLint inputAttrib = glGetAttribLocation(program, "inValue");
    glEnableVertexAttribArray(inputAttrib);
    glVertexAttribPointer(inputAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);

Transform feedback will return the values of `outValue`, but first we'll need to
create a VBO to hold these, just like the input vertices:我们创建一个VBO来接受结果：

    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_ARRAY_BUFFER, tbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), nullptr, GL_STATIC_READ);

Notice that we now pass a `nullptr` to create a buffer big enough to hold all of
the resulting floats, but without specifying any initial data. The appropriate
usage type is now `GL_STATIC_READ`, which indicates that we intend OpenGL to
write to this buffer and our application to read from it. (See [reference](http://www.opengl.org/sdk/docs/man/xhtml/glBufferData.xml) for usage types)这里注意，在指明数据时传递了nullptr，并且usage为GL_STATIC_READ，这表明我们要OpenGL把数据写到这个VBO中，然后我们CPU从里面读出来。

We've now made all preparations for the <del>rendering</del> computation
process. As we don't intend to draw anything, the rasterizer should be disabled:现在让我们开始计算。因为我们不再画任何东西，所以我们可以把光栅化器关掉。

    glEnable(GL_RASTERIZER_DISCARD);

To actually bind the buffer we've created above as transform feedback buffer,
we have to use a new function called `glBindBufferBase`.要激活我们的接受结果的TBO，我们要一个不同的bind函数：

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

The first parameter is currently required to be `GL_TRANSFORM_FEEDBACK_BUFFER`
to allow for future extensions. The second parameter is the index of the output
variable, which is simply `0` because we only have one. The final parameter
specifies the buffer object to bind.

Before doing the draw call, you have to enter transform feedback mode:

    glBeginTransformFeedback(GL_POINTS);

It certainly brings back memories of the old `glBegin` days! Just like the
geometry shader in the last chapter, the possible values for the primitive mode
are a bit more limited.

- `GL_POINTS` &mdash; `GL_POINTS`
- `GL_LINES` &mdash; `GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP, GL_LINES_ADJACENCY, GL_LINE_STRIP_ADJACENCY`
- `GL_TRIANGLES` &mdash; `GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY`

If you only have a vertex shader, as we do now, the primitive *must* match the
one being drawn:

    glDrawArrays(GL_POINTS, 0, 5);

Even though we're now working with data, the single numbers can still be seen as
separate "points", so we use that primitive mode.

End the transform feedback mode:

    glEndTransformFeedback();

Normally, at the end of a drawing operation, we'd swap the buffers to present
the result on the screen. We still want to make sure the rendering operation has
finished before trying to access the results, so we flush OpenGL's command
buffer:

    glFlush();

Getting the results back is now as easy as copying the buffer data back to an
array:

    GLfloat feedback[5];
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

If you now print the values in the array, you should see the square roots of the
input in your terminal:

    printf("%f %f %f %f %f\n", feedback[0], feedback[1], feedback[2], feedback[3], feedback[4]);

<img src="../../media/img/c8_basic.png" alt="Basic result" />

Congratulations, you now know how to make your GPU perform general purpose
tasks with vertex shaders! Of course a real GPGPU framework like [OpenCL](http://en.wikipedia.org/wiki/OpenCL)
is generally better at this, but the advantage of transform feedback is that you
can directly repurpose the data in drawing operations, by for example binding
the transform feedback buffer as array buffer and performing normal drawing
calls.

If you have a graphics card and driver that supports it, you could also use
[compute shaders](http://www.opengl.org/wiki/Compute_Shader) in OpenGL 4.3
instead, which were actually designed for tasks that are less related to drawing.

You can find the full code [here](/content/code/c8_basic.txt).

Feedback transform and geometry shaders
=======================================

When you include a geometry shader, the transform feedback operation will
capture the outputs of the geometry shader instead of the vertex shader. For
example:

    // Vertex shader
    const GLchar* vertexShaderSrc = GLSL(
        in float inValue;
        out float geoValue;

        void main() {
            geoValue = sqrt(inValue);
        }
    );

    // Geometry shader
    const GLchar* geoShaderSrc = GLSL(
        layout(points) in;
        layout(triangle_strip, max_vertices = 3) out;

        in float[] geoValue;
        out float outValue;

        void main() {
            for (int i = 0; i < 3; i++) {
                outValue = geoValue[0] + i;
                EmitVertex();
            }

            EndPrimitive();
        }
    );

The geometry shader takes a point processed by the vertex shader and generates
2 more to form a triangle with each point having a 1 higher value.

    GLuint geoShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geoShader, 1, &geoShaderSrc, nullptr);
    glCompileShader(geoShader);

    ...

    glAttachShader(program, geoShader);

Compile and attach the geometry shader to the program to start using it.

    const GLchar* feedbackVaryings[] = { "outValue" };
    glTransformFeedbackVaryings(program, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

Although the output is now coming from the geometry shader, we've not changed
the name, so this code remains unchanged.

Because each input vertex will generate 3 vertices as output, the transform
feedback buffer now needs to be 3 times as big as the input buffer:

    glBufferData(GL_ARRAY_BUFFER, sizeof(data) * 3, nullptr, GL_STATIC_READ);

When using a geometry shader, the primitive specified to
`glBeginTransformFeedback` must match the output type of the geometry shader:

    glBeginTransformFeedback(GL_TRIANGLES);

Retrieving the output still works the same:

    // Fetch and print results
    GLfloat feedback[15];
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);

    for (int i = 0; i < 15; i++) {
        printf("%f\n", feedback[i]);
    }

<img src="../../media/img/c8_geometry.png" alt="Basic geometry shader result" />

Although you have to pay attention to the feedback primitive type and the size
of your buffers, adding a geometry shader to the equation doesn't change much
other than the shader responsible for output.

The full code can be found [here](/content/code/c8_geometry.txt).

Variable feedback
=================

As we've seen in the previous chapter, geometry shaders have the unique property
to generate a variable amount of data. Luckily, there are ways to keep track of
how many primitives were written by using *query objects*.

Just like all the other objects in OpenGL, you'll have to create one first:

    GLuint query;
    glGenQueries(1, &query);

Then, right before calling `glBeginTransformFeedback`, you have to tell OpenGL
to keep track of the number of primitives written:

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);

After `glEndTransformFeedback`, you can stop "recording":

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

Retrieving the result is done as follows:

    GLuint primitives;
    glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);

You can then print that value along with the other data:

    printf("%u primitives written!\n\n", primitives);

<img src="../../media/img/c8_query.png" alt="Query result" />

Notice that it returns the number of primitives, not the number of vertices.
Since we have 15 vertices, with each triangle having 3, we have 5 primitives.

Query objects can also be used to record things such as `GL_PRIMITIVES_GENERATED`
when dealing with just geometry shaders and `GL_TIME_ELAPSED` to measure time
spent on the server (graphics card) doing work.

See [the full code](/content/code/c8_final.txt) if you got stuck somewhere on the way.

Conclusion
==========

You now know enough about geometry shaders and transform feedback to make your
graphics card do some very interesting work besides just drawing! You can even
combine transform feedback and rasterization to update vertices and draw them
at the same time!

变换反馈，导致我们可以从渲染管线中抓取一些结果回来，这使得使用GPU为我们进行普适计算成为可能。

Exercises
=========

- Try writing a vertex shader that simulates gravity to make points hover around
the mouse cursor using transform feedback to update the vertices
