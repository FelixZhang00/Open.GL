纹理对象及其参数
========

就和VBO及VAO一样，纹理也是对象，它必须先通过函数来创建。

	GLuint tex;
	glGenTextures(1, &tex);


纹理一般而言是为了装饰3D模型的，但是实际上纹理可以用来传输多种不同的数据。OpenGL接受1D、2D及3D纹理，我们可以通过这些纹理来存储大量数据在显卡上。一个例子就是把纹理用来存储terrain数据。本文将着重描述如何使用图片纹理，而不是对于所有纹理种类的通用原则。

	glBindTexture(GL_TEXTURE_2D, tex);

和其他对象一样，纹理必须被绑定到相关的操作。因为我们的图片是2D的，所以我们使用GL_TEXTURE_2D目标。

纹理的像素将被纹理坐标引用。这个坐标在[0.0, 1.0]。0，0在左下角，而1，1在右上角。从纹理中读取某个坐标的颜色信息的过程叫做采样。有很多不同的采样方法，不同的方法适用于不同的场景。OpenGL提供了很多参数来控制这个采样过程，下面小节会介绍常用的方式。

Wrapping
--------

The first thing you'll have to consider is how the texture should be sampled when a coordinate outside the range of `0` to `1` is given. OpenGL offers 4 ways of handling this:首先要考虑的是，当给定了坐标后，我们如何从纹理中获取色彩。OpenGL给了4个方式：

- `GL_REPEAT`: The integer part of the coordinate will be ignored and a repeating pattern is formed.整数部分的坐标将会被忽略，一个重复的过程将会被产生；
- `GL_MIRRORED_REPEAT`: The texture will also be repeated, but it will be mirrored when the integer part of the coordinate is odd.纹理会重复，但是对于整数部分是奇数的部分是镜像的；
- `GL_CLAMP_TO_EDGE`: 坐标会被简单的剪切到0.0，1.0；
- `GL_CLAMP_TO_BORDER`: 在[0.0,1.0]之外的部分会被简单的置成某个颜色；

这些解释可能还是有些神秘，因为OpenGL是关于图形的，所以我们可以看下下图的效果：

<img src="../../media/img/c3_clamping.png" alt="" />

采样的每个坐标都是可以设置的，vec3在纹理坐标中为(s,t,r)。纹理参数函数则是glTextParameter*:

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

和以前一样，这里的i表面将设置的值的类型。如果你使用GL_CLAMP_TO_BORDER，并且你希望设置边界的颜色，你可以按如下代码设置这个颜色：

	float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

这个代码会设置边界颜色为红色。

Filtering
--------

Since texture coordinates are resolution independent, they won't always match a pixel exactly. This happens when a texture image is stretched beyond its original size or when it's sized down. OpenGL offers various methods to decide on the sampled color when this happens. This process is called filtering and the following methods are available:因为纹理坐标是分辨率无关的，所以他们并不会永远刚好对应到像素点。这种情况在纹理图片被撑大时会出现。OpenGL提供了多个方法来决定当这种情况发生时采样的方式。这个过程叫做filtering，下面是filtering的6中方式：

- `GL_NEAREST`: Returns the pixel that is closest to the coordinates.返回离坐标最近的像素；
- `GL_LINEAR`: Returns the weighted average of the 4 pixels surrounding the given coordinates.返回周边4个像素的权重平均；
- `GL_NEAREST_MIPMAP_NEAREST`, `GL_LINEAR_MIPMAP_NEAREST`, `GL_NEAREST_MIPMAP_LINEAR`, `GL_LINEAR_MIPMAP_LINEAR`: 在Mipmap模式下使用的采样方式。

Before discussing mipmaps, let's first see the difference between nearest and linear interpolation. The original image is 16 times smaller than the rectangle it was rasterized on.在讨论mipmap之前，我们先来看下nearest和linear 插值。原始图片比需要贴图到的面积小16倍：

<img src="../../media/img/c3_filtering.png" alt="" />

While linear interpolation gives a smoother result, it isn't always the most ideal option. Nearest neighbour interpolation is more suited in games that want to mimic 8 bit graphics, because of the pixelated look.线性插值在这里有一个更平滑的表现。但它并不是永远最佳的。有时候，临近插值在游戏中更实用，特别是在像素风格游戏中。

You can specify which kind of interpolation should be used for two separate cases: scaling the image down and scaling the image up. These two cases are identified by the keywords `GL_TEXTURE_MIN_FILTER` and `GL_TEXTURE_MAG_FILTER`.你可以指定在放大和缩小纹理时使用的不同的采样方式：

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//放大

As you've seen, there is another way to filter textures: mipmaps. Mipmaps are smaller copies of your texture that have been sized down and filtered in advance. It is recommended that you use them because they result in both a higher quality and higher performance.如你所见，还有另外一个filtering方法，即mipmaps。Mipmaps是一些列逐渐缩小的预先filtered的纹理序列。我们强烈推荐你使用它，因为它在效果和性能上都有不错表现：

	glGenerateMipmap(GL_TEXTURE_2D);

Generating them is as simple as calling the function above, so there's no excuse for not using them! Note that you *do* have to load the texture image itself before mipmaps can be generated from it.要生成Mipmap是十分方便的，你只要调用上面的方法就行了。不过在调用这个方法之前，你必须先要将纹理数据传到显卡。

To use mipmaps, select one of the four mipmap filtering methods.要使用mipmaps，我们需要选择合适的mipmap filtering方式：

- `GL_NEAREST_MIPMAP_NEAREST`: Uses the mipmap that most closely matches the size of the pixel being textured and samples with nearest neighbour interpolation.
- `GL_LINEAR_MIPMAP_NEAREST`: Samples the closest mipmap with linear interpolation.
- `GL_NEAREST_MIPMAP_LINEAR`: Uses the two mipmaps that most closely match the size of the pixel being textured and samples with nearest neighbour interpolation.
- `GL_LINEAR_MIPMAP_LINEAR`: Samples closest two mipmaps with linear interpolation.
第二个类型表示，选择序列mipmap中的某一个纹理时使用的方法。第一个类型表示，从此纹理中采样所用的方法。
还有其他一些纹理参数，它们可以被用来完成一些特殊的操作，你可以阅读[specification](http://docs.gl/gl3/glTexParameter).

加载纹理数据
========

Now that the texture object has been configured it's time to load the texture image. This is done by simply loading an array of pixels into it:现在纹理对象已经被设置好了，现在可以加载纹理数据了。这其实就是加载一个像素数组：

	// Black/white checkerboard
	float pixels[] = {
		0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);

The first parameter after the texture target is the *level-of-detail*, where `0` is the base image. This parameter can be used to load your own mipmap images. The second parameter specifies the internal pixel format, the format in which pixels should be stored on the graphics card. Many different formats are available, including compressed formats, so it's certainly worth taking a look at all of the options. The third and fourth parameters specify the width and height of the image. The fifth parameter should always have a value of `0` per the [specification](http://docs.gl/gl3/glTexImage2D). The next two parameter describe the format of the pixels in the array that will be loaded and the final parameter specifies the array itself. The function begins loading the image at coordinate `(0,0)`, so pay attention to this.第一个参数是加载到的纹理目标。第二个参数是细节层次，0表示基础图片。这可以用来加载自己制作的mipmap。第三个参数指明颜色数据格式。有很多不同的可用格式，包括压缩格式，所以这值得我们一探究竟。第四个和第五个参数指明图片的宽和高。第六个参数应该永远被置为0.接下来的2各参数表明数组中的数据格式。最后的参数是数组本身。

But how is the pixel array itself established? Textures in graphics applications will usually be a lot more sophisticated than simple patterns and will be loaded from files. Best practice is to have your files in a format that is natively supported by the hardware, but it may sometimes be more convenient to load textures from common image formats like JPG and PNG. Unfortunately OpenGL doesn't offer any helper functions to load pixels from these image files, but that's where third-party libraries come in handy again! The SOIL library will be discussed here along with some of the alternatives.但是这个纹理数据数组本身是如何建立起来的呢？图形程序一般会使用更高级的数据格式，并且这些纹理一般都是从文件中加载的。最佳的方式是，使用一种被硬件支持的文件格式，把纹理存在里面。但是有时候，我们把纹理保持在通用的图片类型中可能会更加方便，比如JPG和PNG格式。但是不幸的是，OpenGL并没有提供任何从图片加载纹理数据的辅助函数，然而在外面有很多好用的第三方库可以完成这个任务。我们这里使用SOIL（simple opengl image library）来加载图形数据。

SOIL
--------

[SOIL](http://www.lonesock.net/soil.html) (Simple OpenGL Image Library) is a small and easy-to-use library that loads image files directly into texture objects or creates them for you. You can start using it in your project by linking with `SOIL` and adding the `src` directory to your include path. It includes Visual Studio project files to compile it yourself.SOIL是一个简单好用的加载图片的库。

Although SOIL includes functions to automatically create a texture from an image, it uses features that aren't available in modern OpenGL. Because of this we'll simply use SOIL as image loader and create the texture ourselves.虽然SOIL具有自动加载图片生成纹理对象的功能。但是由于它在实现这个功能时，使用了旧世界的OpenGL函数，所以我们在这里仅把它用来加载图片数据。

	int width, height;
	unsigned char* image =
		SOIL_load_image("img.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				  GL_UNSIGNED_BYTE, image);

You can start configuring the texture parameters and generating mipmaps after this.你可以在此之后配置纹理参数和生成Mipmap。

	SOIL_free_image_data(image);

You can clean up the image data right after you've loaded it into the texture.在使用完纹理数据后，你可以释放他们。

>As mentioned before, OpenGL expects the first pixel to be located in the bottom-left corner, which means that textures will be flipped when loaded with SOIL directly. To counteract that, the code in the tutorial will use flipped Y coordinates for texture coordinates from now on. That means that `0, 0` will be assumed to be the top-left corner instead of the bottom-left. This practice might make texture coordinates more intuitive as a side-effect.就如上面所述的，OpenGL期待第一个像素位于左下角，这也意味着当SOIL加载图片时，Y坐标是不对的。为了面对这个问题，代码会翻转Y坐标。这也就意味着0，0将被默认为是左上角而不是左下角。这个方式会使得纹理坐标更符合直觉。

其他选项
--------

Other libraries that support a wide range of file types like SOIL are [DevIL](http://openil.sourceforge.net/) and [FreeImage](http://freeimage.sourceforge.net/). If you're just interested in one file type, it's also possible to use libraries like [libpng](http://www.libpng.org/pub/png/libpng.html) and [libjpeg](http://libjpeg.sourceforge.net/) directly. If you're looking for more of an adventure, have a look at the specification of the [BMP](http://en.wikipedia.org/wiki/BMP_file_format) and [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) file formats, it's not that hard to implement a loader for them yourself.

使用一个纹理
========

As you've seen, textures are sampled using texture coordinates and you'll have to add these as attributes to your vertices. Let's modify the last sample from the previous chapter to include these texture coordinates. The new vertex array will now include the `s` and `t` coordinates for each vertex:如你所见，纹理将依据纹理坐标而被采样。我们需要增加纹理坐标到我们的顶点数组中。我们来修改一下原来的顶点数组。新的顶点数组的每个顶点将包含纹理坐标S,T。

	float vertices[] = {
	//  Position	  Color             Texcoords
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
	};

The vertex shader needs to be modified so that the texture coordinates are interpolated over the fragments:顶点着色器也要被修改，它必须将纹理坐标向下传递给片段着色器：

	...

	in vec2 texcoord;

	out vec3 Color;
	out vec2 Texcoord;

	...

	void main()
	{
		Texcoord = texcoord;


Just like when the color attribute was added, the attribute pointers need to be adapted to the new format:就像前面添加颜色attribute一样，attribute在这里也要被重新设置：

	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
						   7*sizeof(float), 0);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
						   7*sizeof(float), (void*)(2*sizeof(float)));

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
						   7*sizeof(float), (void*)(5*sizeof(float)));

As two floats were added for the coordinates, one vertex is now 7 floats in size and the texture coordinate attribute consists of 2 of those floats.因为新增了2个float作为纹理坐标，所以现在每个顶点具有7个float。

Now just one thing remains: providing access to the texture in the fragment shader to sample pixels from it. This is done by adding a uniform of type `sampler2D`, which will have a default value of 0. This only needs to be changed when access has to be provided to multiple textures, which will be considered in the next section.现在只剩最有一件事情了，那就是在片段着色器中，依照纹理坐标从纹理中取样，并且赋给最终色彩中。

For this sample, the [image of the kitten](/content/code/sample.png) used above will be loaded using the SOIL library. Make sure that it is located in the working directory of the application.在这个例子里，我们使用一只小猫纹理。

	int width, height;
	unsigned char* image =
		SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				  GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

To sample a pixel from a 2D texture using the sampler, the function `texture` can be called with the relevant sampler and texture coordinate as parameters. We'll also multiply the sampled color with the color attribute to get an interesting effect. Your fragment shader will now look like this:我们在片段着色器中使用texture函数来从纹理中采样，这里采样的结果加上了原来的色彩。

	#version 150

	in vec3 Color;
	in vec2 Texcoord;

	out vec4 outColor;

	uniform sampler2D tex;

	void main()
	{
	    outColor = texture(tex, Texcoord) * vec4(Color, 1.0);
	}

When running this application, you should get the following result:在运行这个后，你应该会得到如下的效果：

<img src="../../media/img/c3_window.png" alt="" />

If you get a black screen, make sure that your shaders compiled successfully and that the image is correctly loaded. If you can't find the problem, try comparing your code to the [sample code](/content/code/c3_basic.txt).

Texture units纹理单元
========

The sampler in your fragment shader is bound to texture unit `0`. Texture units are references to texture objects that can be sampled in a shader. Textures are bound to texture units using the `glBindTexture` function you've used before. Because you didn't explicitly specify which texture unit to use, the texture was bound to `GL_TEXTURE0`. That's why the default value of `0` for the sampler in your shader worked fine.在我们上面代码的片段着色器中，sampler被默认的设置为0。纹理单元引用到可以在着色器中被采样的纹理对象。纹理被绑定到纹理对象使用glBindTexture。因为我们没有显式的指明哪个纹理单元将被使用，所以默认的会使用GL_TEXTURE0。这也就是为什么片段着色器中我们没有设置tex采样器，也运行的良好，因为它刚好是0即GL_TEXTURE0。

The function `glActiveTexture` specifies which texture unit a texture object is bound to when `glBindTexture` is called.函数glActiveTexture指明了在使用glBindTexture时，哪个纹理单元将被激活。

	glActiveTexture(GL_TEXTURE0);

The amount of texture units supported differs per graphics card, but it will be at least 48. It is safe to say that you will never hit this limit in even the most extreme graphics applications.不同的显卡有不同的纹理单元的，但是它至少会有48个。

To practice with sampling from multiple textures, let's try blending the images of the kitten and [one of a puppy](/content/code/sample2.png) to get the best of both worlds! Let's first modify the fragment shader to sample from two textures and blend the pixels:下面演示从多个纹理中采样，我们在这里把一只狗和一只猫混合起来：

	...

	uniform sampler2D texKitten;
	uniform sampler2D texPuppy;

	void main()
	{
		vec4 colKitten = texture(texKitten, Texcoord);
		vec4 colPuppy = texture(texPuppy, Texcoord);
	    outColor = mix(colKitten, colPuppy, 0.5);
	}

The `mix` function here is a special GLSL function that linearly interpolates between two variables based on the third parameter. A value of `0.0` will result in the first value, a value of `1.0` will result in the second value and a value in between will result in a mixture of both values. You'll have the chance to experiment with this in the exercises.mix是一个GLSL提供的线性混合函数，它会根据第三个参数来混合前两个参数。

Now that the two samplers are ready, you'll have to assign the first two texture units to them and bind the two textures to those units. This is done by adding the proper `glActiveTexture` calls to the texture loading code.现在片段着色器中的两个采样器都已经就绪，我们现在要在将两个纹理单元分别赋给它们，同时还要将纹理绑定到这两个纹理单元：

	GLuint textures[2];
	glGenTextures(2, textures);

	int width, height;
	unsigned char* image;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	image = SOIL_load_image("sample.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				  GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	image = SOIL_load_image("sample2.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
				  GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

The texture units of the samplers are set using the `glUniform` function you've seen in the previous chapter. It simply accepts an integer specifying the texture unit. This code should result in the following image.我们使用glUniform来设置在着色器中的uniform变量。

<img src="../../media/img/c3_window2.png" alt="" />

As always, have a look at the sample [source code](/content/code/c3_multitexture.txt) if you have trouble getting the program to work.

Now that texture sampling has been covered in this chapter, you're finally ready to dive into transformations and ultimately 3D. The knowledge you have at this point should be sufficient for producing most types of 2D games, except for transformations like rotation and scaling which will be covered in the [next chapter](/transformations).

Exercises
========

- Animate the blending between the textures by adding a `time` uniform. ([Solution](/content/code/c3_exercise_1.txt))
- Draw a reflection of the kitten in the lower half of the rectangle. ([Solution](/content/code/c3_exercise_2.txt))
- Now try adding distortion with `sin` and the time variable to simulate water. ([Expected result](../../media/img/c3_window3.png), [Solution](/content/code/c3_exercise_3.txt))
