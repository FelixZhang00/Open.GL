其它的缓冲区
========
直到现在为止，我们只只用带了一个输出缓冲区，即 颜色缓冲区（color buffer）。这一章将介绍另外两个常用的缓冲区，本别是*深度缓冲区*和*模板缓冲区*。我们会举例说明它们各自的作用。



准备工作
========

To best demonstrate the use of these buffers, let's draw a cube instead of a flat shape. The vertex shader needs to be modified to accept a third coordinate:为了更好的演示这两个缓冲区的作用，我们先绘制一个正方体而不是先前的平面。那么顶点着色器要能够介绍3D坐标。

	in vec3 position;
	...
	gl_Position = proj * view * model * vec4(position, 1.0);

We're also going to need to alter the color again later in this chapter, so make sure the fragment shader multiplies the texture color by the color attribute:我们还需要修改片段着色器中的色彩，以及相应地attribute设置：

	vec4 texColor = mix(texture(texKitten, Texcoord),
						 texture(texPuppy, Texcoord), 0.5);
	outColor = vec4(Color, 1.0) * texColor;

Vertices are now 8 floats in size, so you'll have to update the vertex attribute offsets and strides as well. Finally, add the extra coordinate to the vertex array:现在每个顶点有8个float了，所以你也得更新一下attribute设置。

	float vertices[] = {
	  // X      Y     Z     R     G     B     U     V
		-0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f
	};

Confirm that you've made all the required changes by running your program and checking if it still draws a flat spinning image of a kitten blended with a puppy. A single cube consists of 36 vertices (6 sides * 2 triangles * 3 vertices), so I will ease your life by providing the array [here](/content/code/c5_vertices.txt).我们的正方体有6个面，在使用glDrawArrays来绘制时需要36个顶点，我们这里不适用EBO，而是使用36个顶点，这有点麻烦，所以你可以直接从[here](/content/code/c5_vertices.txt)复制顶点数组。

	glDrawArrays(GL_TRIANGLES, 0, 36);

We will not make use of element buffers for drawing this cube, so you can use `glDrawArrays` to draw it. If you were confused by this explanation, you can compare your program to [this reference code](/content/code/c5_cube.txt).如上所述，我们不使用EBO来绘制这个正方体，所以我们仍旧需要使用glDrawArrays来绘制它。如果你遇到问题，你可以看代码[this reference code](/content/code/c5_cube.txt).

<div class="livedemo_wrap">
	<div class="livedemo" id="demo_c5_cube" style="background: url('../../media/img/c5_window.png')">
		<canvas width="640" height="480"></canvas>
		<script type="text/javascript" src="/content/demos/c5_cube.js"></script>
	</div>
</div>

It immediately becomes clear that the cube is not rendered as expected when seeing the output. The sides of the cube are being drawn, but they overlap each other in strange ways! The problem here is that when OpenGL draws your cube triangle-by-triangle, it will simply write over pixels even though something else may have been drawn there before. In this case OpenGL will happily draw triangles in the back over triangles at the front.很明显，正方体没有按预期的那样被绘制，各个面好像交错在一起，非常奇怪。这是因为OpenGL在这里仅是简单地按顶点顺序绘图，而没有处理三角形之间的遮挡关系。在这种情况下，Opengl可能会把在后面的三角形画到在前面的三角形之上。

Luckily OpenGL offers ways of telling it when to draw over a pixel and when not to. I'll go over the two most important ways of doing that, depth testing and stencilling, in this chapter.幸运地是，OpenGL提供了2个方法来解决这个问题。它们分别是深度测试和模板测试。

深度缓冲区
========

*Z-buffering* is a way of keeping track of the depth of every pixel on the screen. The depth is proportional to the distance between the screen plane and a fragment that has been drawn. That means that the fragments on the sides of the cube further away from the viewer have a higher depth value, whereas fragments closer have a lower depth value.Z-buffering是一个记录屏幕上每个像素的深度位置的方法。所谓的深度是指屏幕平面到片段的距离。这意味着，正方体的远处的面比近处的要具有更高的深度值，而较近的片段则具有一个较低的深度值。

If this depth is stored along with the color when a fragment is written, fragments drawn later can compare their depth to the existing depth to determine if the new fragment is closer to the viewer than the old fragment. If that is the case, it should be drawn over and otherwise it can simply be discarded. This is known as *depth testing*.如果深度伴随着颜色一起被记录了，那么后来的片段会先比较已经存在的深度，如果新片段的深度比原来的深度低，那么新片段将覆盖或者混合前面的片段，否则将忽略。这个过程叫做深度测试。

OpenGL offers a way to store these depth values in an extra buffer, called the *depth buffer*, and perform the required check for fragments automatically. The fragment shader will not run for fragments that are invisible, which can have a significant impact on performance. This functionality can be enabled by calling `glEnable`.OpenGL提供了一个存储这些深度信息的buffer。它叫做depth buffer，即深度缓冲区。它会自动的进行片段的深度检查。对于不可见的片段，我们的片段着色器不会执行。这会显著地提高性能。这个功能可以通过glEnable来打开。

	glEnable(GL_DEPTH_TEST);

If you enable this functionality now and run your application, you'll notice that you get a black screen. That happens because the depth buffer is filled with 0 depth for each pixel by default. Since no fragments will ever be closer than that they are all discarded.如果你现在打开这个功能并运行程序，你会得到一个黑色的屏幕。这是因为深度缓冲区在默认状态下是全0填充的，这回导致没有任何片段会被魂之。

The depth buffer can be cleared along with the color buffer by extending the `glClear` call:深度缓冲区可以像颜色缓冲区一样被事先清理：

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

The default clear value for the depth is `1.0f`, which is equal to the depth of your far clipping plane and thus the furthest depth that can be represented. All fragments will be closer than that, so they will no longer be discarded. 默认的清理填充为1.0f，这与我们的远处的clipping平面一直，它是我们能够表现的最远距离。所有的片段都应该比它近，才不会被忽略。

<div class="livedemo_wrap">
	<div class="livedemo" id="demo_c5_depth" style="background: url('../../media/img/c5_window2.png')">
		<canvas width="640" height="480"></canvas>
		<script type="text/javascript" src="/content/demos/c5_depth.js"></script>
	</div>
</div>

With the depth test capability enabled, the cube is now rendered correctly. Just like the color buffer, the depth buffer has a certain amount of bits of precision which can be specified by you. Less bits of precision reduce the extra memory use, but can introduce rendering errors in more complex scenes.在这些工作之后，我们的正方体就能够被正确地渲染了。和颜色缓冲区一样，深度缓冲区也有一些比特位来配置它的精度。使用较低精度的深度缓冲区，可以减少内存的小号，但是可能会在复杂场景时出现一些渲染错误。

模板缓冲区
========

The stencil buffer is an optional extension of the depth buffer that gives you more control over the question of which fragments should be drawn and which shouldn't. Like the depth buffer, a value is stored for every pixel, but this time you get to control when and how this value changes and when a fragment should be drawn depending on this value. Note that if the depth test fails, the stencil test no longer determines whether a fragment is drawn or not, but these fragments can still affect values in the stencil buffer!模板缓冲区，和深度缓冲区一样，模板缓冲区内对每个像素存有一个值，但是这次我们能够控制这个值的变更，不像深度测试是自动完成的一样，模板的测试过程完全是我们控制的。但注意，如果深度测试失败了，那么模板测试将不再决定片段是否绘制，但是这个片段依旧会隐形模板缓冲区的值。

To get a bit more acquainted with the stencil buffer before using it, let's start by analyzing a simple example.为了在使用模板缓冲区之前更多的了解它，我们来分析一个例子：

<img src="../../media/img/c5_stencil.png" alt="" />

In this case the stencil buffer was first cleared with zeroes and then a rectangle of ones was drawn to it. The drawing operation of the cube uses the values from the stencil buffer to only draw fragments with a stencil value of 1.在这个例子中，模板缓冲区一开始被清理为全0，后来其中的一个方形被画成全1，此时只有这部分才会绘制了。

Now that you have an understanding of what the stencil buffer does, we'll look at the relevant OpenGL calls.现在你应该对模板缓冲区，以及为什么叫模板，有了一个理解了。现在我们来看一下相关的函数：

	glEnable(GL_STENCIL_TEST);

Stencil testing is enabled with a call to `glEnable`, just like depth testing. You don't have to add this call to your code just yet. I'll first go over the API details in the next two sections and then we'll make a cool demo.模板测试通过glEnable函数来打开，这与深度测试是类似的。你现在还不用把这句代码写到我们的工程里，我们先来看下其它的相关API，之后我们在看一个酷毙了的例子。

设置模板值
--------
（下面这部分还是看英文吧，翻译成中文了，很难准确表达）

Regular drawing operations are used to determine which values in the stencil buffer are affected by any stencil operation. If you want to affect a rectangle of values like in the sample above, simply draw a 2D quad in that area. What happens to those values can be controlled by you using the `glStencilFunc`, `glStencilOp` and `glStencilMask` functions.

The `glStencilFunc` call is used to specify the conditions under which a fragment passes the stencil test. Its parameters are discussed below.

- `func`: The test function, can be `GL_NEVER`, `GL_LESS`, `GL_LEQUAL`, `GL_GREATER`, `GL_GEQUAL`, `GL_EQUAL`, `GL_NOTEQUAL`, and `GL_ALWAYS`.
- `ref`: A value to compare the stencil value to using the test function.
- `mask`: A bitwise AND operation is performed on the stencil value and reference value with this mask value before comparing them.

If you don't want stencils with a value lower than 2 to be affected, you would use:

	glStencilFunc(GL_GEQUAL, 2, 0xFF);

The mask value is set to all ones (in case of an 8 bit stencil buffer), so it will not affect the test.

The `glStencilOp` call specifies what should happen to stencil values depending on the outcome of the stencil and depth tests. The parameters are:

- `sfail`: Action to take if the stencil test fails.
- `dpfail`: Action to take if the stencil test is successful, but the depth test failed.
- `dppass`: Action to take if both the stencil test and depth tests pass.

Stencil values can be modified in the following ways:

- `GL_KEEP`: The current value is kept.
- `GL_ZERO`: The stencil value is set to 0.
- `GL_REPLACE`: The stencil value is set to the reference value in the `glStencilFunc` call.
- `GL_INCR`: The stencil value is increased by 1 if it is lower than the maximum value.
- `GL_INCR_WRAP`: Same as `GL_INCR`, with the exception that the value is set to 0 if the maximum value is exceeded.
- `GL_DECR`: The stencil value is decreased by 1 if it is higher than 0.
- `GL_DECR_WRAP`: Same as `GL_DECR`, with the exception that the value is set to the maximum value if the current value is 0 (the stencil buffer stores unsigned integers).
- `GL_INVERT`: A bitwise invert is applied to the value.

Finally, `glStencilMask` can be used to control the bits that are written to the stencil buffer when an operation is run. The default value is all ones, which means that the outcome of any operation is unaffected.

If, like in the example, you want to set all stencil values in a rectangular area to 1, you would use the following calls:

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);

In this case the rectangle shouldn't actually be drawn to the color buffer, since it is only used to determine which stencil values should be affected.

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

The `glColorMask` function allows you to specify which data is written to the color buffer during a drawing operation. In this case you would want to disable all color channels (red, green, blue, alpha). Writing to the depth buffer needs to be disabled separately as well with `glDepthMask`, so that cube drawing operation won't be affected by leftover depth values of the rectangle. This is cleaner than simply clearing the depth buffer again later.

Using values in drawing operations
--------

With the knowledge about setting values, using them for testing fragments in drawing operations becomes very simple. All you need to do now is re-enable color and depth writing if you had disabled those earlier and setting the test function to determine which fragments are drawn based on the values in the stencil buffer.

	glStencilFunc(GL_EQUAL, 1, 0xFF);

If you use this call to set the test function, the stencil test will only pass for pixels with a stencil value equal to 1. A fragment will only be drawn if it passes both the stencil and depth test, so setting the `glStencilOp` is not necessary. In the case of the example above only the stencil values in the rectangular area were set to 1, so only the cube fragments in that area will be drawn.

	glStencilMask(0x00);

One small detail that is easy to overlook is that the cube draw call could still affect values in the stencil buffer. This problem can be solved by setting the stencil bit mask to all zeroes, which effectively disables stencil writing.

镜面反射
========

Let's spice up the demo we have right now a bit by adding a floor with a reflection under the cube. I'll add the vertices for the floor to the same vertex buffer the cube is currently using to keep things simple:现在我们在原来的正方体下面增加一面地板，并且地板能够反射正方体的倒影。为了方便，我们把地板的顶点数据加到原来的顶点数组里面。

	float vertices[] = {
		...

		-1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	}

Now add the extra draw call to your main loop:把地板的绘制加到绘图过程中：

	glDrawArrays(GL_TRIANGLES, 36, 6);

To create the reflection of the cube itself, it is sufficient to draw it again but inverted on the Z-axis:要创建反射效果，实际上就是再画一个在Z轴翻转的正方体。

	model = glm::scale(
		glm::translate(model, glm::vec3(0, 0, -1)),
		glm::vec3(1, 1, -1)
	);
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, 36);

I've set the color of the floor vertices to black so that the floor does not display the texture image, so you'll want to change the clear color to white to be able to see it. I've also changed the camera parameters a bit to get a good view of the scene.

<div class="livedemo_wrap">
	<div class="livedemo" id="demo_c5_floor" style="background: url('../../media/img/c5_window3.png')">
		<canvas width="640" height="480"></canvas>
		<script type="text/javascript" src="/content/demos/c5_floor.js"></script>
	</div>
</div>

Two issues are noticeable in the rendered image:

- The floor occludes the reflection because of depth testing.
- The reflection is visible outside of the floor.

The first problem is easy to solve by temporarily disabling writing to the depth buffer when drawing the floor:

	glDepthMask(GL_FALSE);
	glDrawArrays(GL_TRIANGLES, 36, 6);
	glDepthMask(GL_TRUE);

To fix the second problem, it is necessary to discard fragments that fall outside of the floor. Sounds like it's time to see what stencil testing is really worth!

It can be greatly beneficial at times like these to make a little list of the rendering stages of the scene to get a proper idea of what is going on.

- Draw regular cube.
- Enable stencil testing and set test function and operations to write ones to all selected stencils.
- Draw floor.
- Set stencil function to pass if stencil value equals 1.
- Draw inverted cube.
- Disable stencil testing.

The new drawing code looks like this:

	glEnable(GL_STENCIL_TEST);

		// Draw floor
		glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF); // Write to stencil buffer
		glDepthMask(GL_FALSE); // Don't write to depth buffer
		glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)
			
		glDrawArrays(GL_TRIANGLES, 36, 6);

		// Draw cube reflection
		glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
		glStencilMask(0x00); // Don't write anything to stencil buffer
		glDepthMask(GL_TRUE); // Write to depth buffer

		model = glm::scale(
			glm::translate(model, glm::vec3(0, 0, -1)),
			glm::vec3(1, 1, -1)
		);
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisable(GL_STENCIL_TEST);

I've annotated the code above with comments, but the steps should be mostly clear from the stencil buffer section.

Now just one final touch is required, to darken the reflected cube a little to make the floor look a little less like a perfect mirror. I've chosen to create a uniform for this called `overrideColor` in the vertex shader:

	uniform vec3 overrideColor;
	...
	Color = overrideColor * color;

And in the drawing code for the reflected cube

	glUniform3f(uniColor, 0.3f, 0.3f, 0.3f);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	glUniform3f(uniColor, 1.0f, 1.0f, 1.0f);

where `uniColor` is the return value of a `glGetUniformLocation` call.

<div class="livedemo_wrap">
	<div class="livedemo" id="demo_c5_reflection" style="background: url('../../media/img/c5_window4.png')">
		<canvas width="640" height="480"></canvas>
		<script type="text/javascript" src="/content/demos/c5_reflection.js"></script>
	</div>
</div>

Awesome! I hope that, especially in chapters like these, you get the idea that working with an API as low-level as OpenGL can be a lot of fun and pose interesting challenges! As usual, the final code is available [here](/content/code/c5_reflection.txt).

Exercises
========

There are no real exercises for this chapter, but there are a lot more interesting effects you can create with the stencil buffer. I'll leave researching the implementation of other effects, such as [stencil shadows](http://en.wikipedia.org/wiki/Shadow_volume#Stencil_buffer_implementations) and [object outlining](http://www.flipcode.com/archives/Object_Outlining.shtml) as an exercise to you.
