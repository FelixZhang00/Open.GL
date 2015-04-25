渲染管线
========
原Overv 译IICHENBF

想学习OpenGL，那你就得做好艰苦工作的准备。也意味着，你得深入学习图形渲染，但一旦你了解了必要的知识，你就会发现学习OpenGL并不是那么艰难。另外，文末的练习题会让你领会现代图形渲染的基础。

渲染管线涵盖了从输入数据到最终图片的所有步骤。我会在如下的代码中逐一演示。

<img src="../../media/img/c2_pipeline.png" alt="" />

所有的内容都是从*顶点*开始的。这些顶点控制着后面被渲染的图形的形状。每个点除了位置信息之外，还存储着一些属性。要存储哪些属性则是完全由我们决定的。**通常的属性是世界3D坐标和纹理坐标。**

顶点着色器是一个在显卡上运行的小程序。它的输入是各个顶点。在这里进行透视变换，它将图形的3D 世界坐标转换到2D屏幕坐标。而且它还会传输其他的重要的属性，比如颜色和纹理坐标到渲染管线的下一级。

在顶点坐标从3D世界坐标转换到2D的屏幕坐标后，显卡将要由这些顶点构建三角形、线和点。这些形状叫做*图元*，这是因为它们是更复杂图形的基础。我们多个绘图模式可以选择，比如三角形带和条线。这些模式可以减少需要传入的顶点数量，因为一些顶点是不同图元共用的。

接下来是*几何着色器*，它是可选的，并且是最近才加入到OpenGL中的。与顶点着色器不同，几何着色器可以输出比输入更多的数据。它把图元作为输入数据。它可以修改，丢弃，替换，增加这些图元，最后会把图元输出给下一级。因为GPU和PC的其它部分（比如CPU）直接的速度是很慢的，而这个阶段可以做一些额外的工作来帮助你减少由CPU出入的数据。比如像素游戏，你不必把每个像素方格作为一个图元传入，而只传入一些点顶点（point vertex），并且附带世界坐标和颜色以及材质，而最终的方格可以在几何着色器阶段把点处理成方格。


在一系列图形被转化到屏幕坐标后，光栅化器把可见的图形转化成像素大小的*片段*。从顶点着色器和几何着色器传来的顶点属性将被作为片段着色器的输入。如图片所示的，三角形的各个片段平滑的颜色变化，即使只有3个顶点的属性被定义。

片段着色器对每个片段，由其插值属性等，输出最终的颜色。这个过程，通常是通过依照插入的纹理坐标从纹理中采样而来，或者只是简单的产生一个颜色。在更高级的阶段，这里可能会有关于光照和阴影的计算。着色器也可以忽略某些片段，那么这个形状将被透视。

最后，所有片段将进行混合和深度与模板测试。你现在只要知道，这两个阶段用了某些方法来扔掉某些片段或保留某些片段。比如，当一个三角形被另一个三角形掩盖了，那么进的那个三角形才会显示在屏幕上。

现在，你应该大概了解你的显卡是如何将一组顶点渲染成屏幕上的图形的，接下来让我们看看代码。

顶点数据
========
你要知道的第一件事情是，你的显卡需要什么数据才能够把场景正确地绘制出来。如上文所述的，这个数据是顶点属性。顶点属性可以包含很多类型，但往往世界坐标是一个的。无论是2D还是3D的图形，这个世界坐标属性会决定你的图形最后在屏幕上的位置。

> **设备坐标（屏幕坐标）**
>
>当你给出的顶点经过如上所述的管线之后，它们的坐标都会变转换到*设备坐标*。设备的X和Y坐标会被映射到-1和1区间。
>
> <br /><span style="text-align: center; display: block"><img src="../../media/img/c2_dc.png" alt="" style="display: inline" /> <img src="../../media/img/c2_dc2.png" alt="" style="display: inline" /></span><br />
>
> 就像一个表格，最中间的坐标是`(0,0)`。并且Y轴在中间的上面。这就与普通图片的坐标系不同了。普通图片的坐标系是以左上角为0点的。因为这种以中心为原点的坐标系有利于3D计算和保持分辨率。

上面这个三角形的三个顶点分别是`（0，0.5）`,`（0.5， -0.5）`,`（0.5，-0.5）`。顶点是顺时针方向排列的。显然，现在各个顶点的区别仅是坐标的区别。因此这些顶点也只具有坐标属性。因为我们直接传递了设备坐标，所以一个X,Y的坐标就满足了要求。

OpenGL要求你以数组的形式传递所有顶点信息。这在一开始可能有点奇怪。为了理解这个数组的格式，我们来看一些这个三角形的顶点数组大概的样子。

	float vertices[] = {
		 0.0f,  0.5f, // Vertex 1 (X, Y)
		 0.5f, -0.5f, // Vertex 2 (X, Y)
		-0.5f, -0.5f  // Vertex 3 (X, Y)
	};

就如你上面看到的，这个数组只是简单的把所有的顶点属性排列在一起。属性的排列次序是不定的，但对于每个顶点而言必须一致。顶点的顺序也不是一定要序列话的。但如果不是的话，就要求我们额外的提供元素序列（element buffer），这个会在本章的末尾阐述。

下一步，就是把这些顶点数据传递到显卡了。这十分重要，因为显存比内存要快很多，并且你不必在每次场景渲染时都传递这些数据。


这是通过创建一个*Vertex Buffer Object（VBO）*来搞定的：

	GLuint vbo;
	glGenBuffers(1, &vbo); // Generate 1 buffer

显卡的内存是由OpenGL在内部管理的，所以我们只能够获取一个索引到这个内存的整数。`GLuint`只是一个跨平台的`unsigned int`，`GLint`也是类似的。我们需要通过这个索引来操作这个VBO，包括激活它或者销毁它。


在上传实际数据到显卡的VBO之前，我们需要先激活这个VBO，使用`glBindBuffer`来激活这个VBO：

	glBindBuffer(GL_ARRAY_BUFFER, vbo);

就如`GL_ARRAY_BUFFER`这个枚举变量所体现的，OpenGL中还有其它类型的buffer。但是这些其他类型的Buffer在此刻并不是十分重要。这句的意思是把VBO设置为活动的`array buffer`。在这之后，我们的VBO就被激活了，那么我们就能够传递数据给这个VBO了。

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

注意这个函数并没有引用我们上面创建的VBO，而是直接使用活动的array buffer。第二个参数表明数据的大小。最后一个参数是十分重要的。它的值表示了这些个顶点数据的`usage`。下面是与绘图相关的方式：

- `GL_STATIC_DRAW`: The vertex data will be uploaded once and drawn many times (e.g. the world).这些个顶点数据将上传一次，但会被绘制多次（比如世界场景）。
- `GL_DYNAMIC_DRAW`: The vertex data will be changed from time to time, but drawn many times more than that.这些顶点数据会时不时改变，但绘制的次数多于改变的次数。
- `GL_STREAM_DRAW`: The vertex data will change almost every time it's drawn (e.g. user interface).这些顶点数据基本上每次绘制都会改变。

这个usage参数会影响这些顶点数据在显卡中的存储方式和策略，以达到高效。比如，`GL_STREAM_DRAW`会把这些数据存在内存中，这样写这些数据会快一些，不过绘制速度会有稍微影响。

顶点数据已经被传递到显卡了。但它们现在还没有准备好被使用。记住，我们可以以任意的排列方式传递多种不同的顶点属性。现在我们需要让显卡知道我们该如何处理这些顶点属性。这也是现代的显卡流水线所灵活的地方。

着色器
========

如上所述，渲染管线中有3个着色阶段。每个着色阶段有明确的目的，并且在老的OpenGL中，你只能稍微修改一些着色的行为方式。而现代的OpenGL，着色器的行为已经可以由我们完全控制了。这也就是为什么我们可以自由定义顶点的属性及其排列。你必须同时实现顶点和片段着色器，这样才能将图形显示在屏幕上，而几何着色器是可选的，我们稍后再解释它。

着色器是用一种与C风格类似的语言写的，它叫做GLSL（OpenGL Shading Language）。OpenGL会在运行时编译我们的着色器程序，并且把它加载到显卡中。每个版本的OpenGL的着色语言是有差异的，新版本的着色语言会有更多的特性，我们在这里将使用GLSL 1.50。这个版本名称可能有点落后，因为我们的OpenGL是3.2版本的。不过实际上，GLSL和OpenGL不是同时产生的，直到OpenGL2.0时，我们才有了GLSL1.10。

顶点着色器
----------

顶点着色器是显卡中用来处理顶点数组中每个顶点及其顶点属性的程序。它的责任是计算产生顶点的设备坐标并将片段着色器需要的数据传递到下一级。这也是3D坐标转换处理的地方。片段着色器一般需要顶点颜色和纹理坐标，它们一般在顶点这里直接由输入到输出，而不进行额外的处理。

记住，在这里我们在顶点数组里面存放的坐标就是设备坐标，所以我们的顶点着色器根本不需要进行任何的坐标转换。

	#version 150

	in vec2 position;

	void main()
	{
		gl_Position = vec4(position, 0.0, 1.0);
	}

这个预处理标志表示这个GLSL代码使用的GLSL版本。我们在这里使用1.50版本的。下一步，我们表明我们使用的唯一的属性，那就是坐标position。在普通的C类似之外，GLSL有一些内置的vector和matrix类型，它们以`vec*`和`mat*`的格式出现。这些类型的数据往往都是`float`的。在`vec`后的数字，表示这个类型内部有多少个维度。而`mat`后面的数字表示该矩阵的宽度。因为我们的坐标数据仅有x/y两个，所以`vec2`就够用了。

>你可以十分灵活的使用这些顶点数据类型。比如上面vec4的建立，就使用了一个简便的方式。下面句表达同一个意思：
>
>     gl_Position = vec4(position, 0.0, 1.0);
>     gl_Position = vec4(position.x, position.y, 0.0, 1.0);
>
> 当你在处理颜色时，你可以使用`r`,`g`,`b`和`a`，来取代xyzw来使表达更易读。

最后的设备坐标值被赋给了特别的`gl_Position`变量。因为这个坐标将被图元组装和其他内置的过程所使用。为了这些过程的正确性，W的值需要是1.0f。如果不是它，除此之外，我们可以自由的处理这些属性。在本章末我们可以看到更进一步的处理。

片段着色器
-----------

顶点着色器的产出将影响这个图元所覆盖的所有像素。这些像素叫做片段。片段是片段着色器的处理目标。就像顶点着色器有一个法定的产出（gl_Position）一样，片段着色器的法定产出是片段的颜色。这个片段的最终色彩由我们的代码使用顶点颜色、纹理坐标和其他来自顶点着色器的数据来生成。

我们的三角形现在只包含白色的像素点，所以我们的片段着色器简单的把产出颜色设置为白色即可：

	#version 150

	out vec4 outColor;

	void main()
	{
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
	}

你可能马上发现我们并没有像顶点着色器一样使用一个内建的变量来存放最终的结果。这是因为实际上片段着色器是可以输出多个色彩的。在使用着色器时我们会演示这部分。`outColor`变量使用了`vec4`类型，因为每个颜色都包含R\G\B和alpha部分。OpenGL中的颜色都用浮点数来表示(float)，[0.0,1.0]而不是[0,255]。

编译着色器
--------

当你把着色器的代码加载到内存后（我们也可以直接使用编写在代码中的string），编译这个着色器是十分简单的。和顶点buffers一样，我们先要创建一个着色器对象，并把着色器代码发送给它。

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);

与VBO不同，我们可以直接给某个着色器的索引传递数据，而不用事先激活它。`glShaderSource`函数可以使用数组中的多个string，但我们通常把shader代码放在一个char数组中。最后一个参数是一个数组，其各个值应该是对应的各段string的长度。当传NULL时，就默认的把null作为各string的结束符。

剩下的就是把代码编译到显卡可以识别的编码：

	glCompileShader(vertexShader);

注意编译很可能失败，比如发生了语法错误等，但是glGetError将不会报这种错误，我们通过下面这个代码可以查看着色器编译的错误：

> **检查着色器编译是否成功**
>
>     GLint status;
>     glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
>
> 如果status是GL_TRUE，说明编译成功了.
> <br /><br />
> **否则我们可以使用下面代码来查看错误信息**
>
>     char buffer[512];
>     glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
>
> 这回存储首的511byte+null结束符到我们提供的buffer中。注意，即使编译成功，这个log可能还会包含有用的警告信息。所以每次编译，无论编译成功与否都输出此log有助于我们提前发现问题。

The fragment shader is compiled in exactly the same way:片段着色器的编译方法和顶点着色器一样：

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

同样的，请确认编译是否成功。

将着色器组合成着色程序
--------

直到现在，顶点着色器和片段着色器依旧是2个分开的对象，没有被链接起来。要链接他们，我们要先创建一个`program`对象。并将这两个着色器与它进行关联。

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

因为片段着色器允许将结果写到过个buffer中，所以我们要显式地标记哪个变量输出到哪个buffer。这必须在加载(link)着色程序之前完成。然而，因为默认使用0号buffer，并且我们的片段着色器只有一个输出变量，所以下面这句并不是必须的。

	glBindFragDataLocation(shaderProgram, 0, "outColor");

>当要渲染到多个buffer时，请使用glDrawBuffers来绘图，因为默认的只有使用第一个buffer。

在绑定片段和顶点着色器之后，这个链接操作实际上要用link来完成。在着色器加入到某个program之后，它们依旧可以被修改，但是在link之后，实际的program将不会被改变，改变的部分直到再次link才会生效。同时，绑定多个着色器到同一个处理阶段也是允许的，只要它们组合起来能够完成整个shader的工作即可。一个shader对象可以用glDeleteShader函数来删除。但是shader并不会实际的被删除，直到它从所有被关联的program接触关联为止。使用glDetachShader可以解除shader与program的关联。

	glLinkProgram(shaderProgram);

如果要使用这个program，你就要调用这个函数：

	glUseProgram(shaderProgram);

就像VBO一样，任何使用只允许一个program激活。

关联顶点数据
--------

虽然我们现在已经有了顶点数据和着色器，但是OpenGL现在依旧还不知道顶点数据是的格式和排列。我们先从着色程序中找到position变量的为止：

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");

着色程序的变量的位置是一个整数，它与着色程序输入变量的定义次序有关。在这个例子中，首个也是唯一一个输入变量position将在位置0.

现在我们可以告知OpenGL如何从顶点数据中获取position变量的值：

	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

第一个参数是输入变量的位置索引。第二个参数是输入变量的内涵变量数量。第三个参数是数据的类型。第四个变量表明是否需要把输入数据归一化到[-1.0,1.0]（对于float）或者[0.0,f.10]（对于非float）。

最后两个参数十分重要。因为它们确定了此输入变量在顶点数组中的位置。第一个参数表示stride(步距)，即在两个position数据之间间隔了多少byte。值0表示中间没有间隔。这就是现在的情况，因为我们的顶点数组里面只有位置信息，两个位置信息之间没有其它数据。最后一个参数是offset，即我们的输入变量从数组的第几个byte开始。因为我们这里没有其它信息，所以也是0.

注意，这个函数不仅保存了stride和offset，同时它也保存了当前GL_ARRAY_BUFFER激活的VBO。也就是说，我们在drawing时并不一定要激活正确地VBO，因为绘制时的attribute在这里使用glVertexAttribPointer绑定到了此时的激活的VBO了。也就是说，不同的attribute可以关联到不同的VBO。即在一个绘制内，可以使用多个VBO的数据。

如果你不能完全理解上面的意思，也不必担心，因为我们将会看到如何改变这个例子来增加多个attribute：

	glEnableVertexAttribArray(posAttrib);

最后，vertex attribute需要被激活。

Vertex Array Object(VAO)
--------

你也能够想象，真实地图形程序肯定会使用很多个不同的shader和不同的顶点数组结构。改变shader program只要使用glUseProgram即可，但是VBO与shader program之间的attribute关系又要重新设置了。

幸运地是，OpenGL提供了一个方法来存储这些attribute与VBO的关联关系，即Vertex Array Object（VAO）。VAO会存储shader program中的attribute与VBO数据的关联。

下面代码创建一个VAO对象：

	GLuint vao;
	glGenVertexArrays(1, &vao);

要使用它，就要bind它：

	glBindVertexArray(vao);

在bind一个VAO后，下面的glVertexAttribPointer调用所关联的VBO与attribute关系就会存储在这个活动的VAO中，以后要想切换此关联关系，只要bind不同的VAO即可。记住，VAO并不会存储任何的顶点数据，它只记住VBO的索引以及如何从该VBO中获取attribute值。

因为，只有当VAO被bind后，下面的VBO与attribute关系才会记录到此VAO中，所以我们要再调用glVertexAttribPointer之前bind我们的VAO。

绘制
========

现在你已经加载了顶点数据，并且创建了着色程序，并且关联了顶点数据与着色程序的attribute，已经准备好了绘制这个三角形了。因为被用来存储attribute与VBO关联信息的VAO已经被启用了，所以我们不必担心它。所以现在我们就可以调用：

	glDrawArrays(GL_TRIANGLES, 0, 3);

第一个参数指明图元的类型（一般是点、线和三角形），第二个参数指明在一开始需要跳过多少个顶点，最后一个参数指明要绘制多少个顶点。

现在运行这个程序，你会看到如下图形：

<img src="../../media/img/c2_window.png" alt="" />

如果你什么都没看到，请首先确保你的shader都编译通过了，并且shader program链接正确了。其次，arrtibute array被激活了，同时VAO要正确设置了，最后顶点数据是否正确，glGetError是否返回0。如果这些检查后还是无法解决问题，请与[this sample](/content/code/c2_triangle.txt)作比较。

Uniforms
========

到现在为止，我们三角形的颜色还是被硬编码到片段着色器中的，但是如果你要在编译着色器后改变三角形的颜色呢？顶点属性并不是唯一的向着色程序传递数据的方法。另一个方法叫做uniforms。这些uniform可以理解为全局变量，与attribute不同，它们对所有的vertex和fragment具有相同的值。为了演示如何使用它们，我们在这里使用一个uniform来改变三角形的颜色。

在片段着色器里面使用一个uniform：

	#version 150

	uniform vec3 triangleColor;

	out vec4 outColor;

	void main()
	{
		outColor = vec4(triangleColor, 1.0);
	}

输出颜色的最后一个值是alpha，也就是透明度。如果你现在运行程序，你会看到三角形是黑色的，这是因为triangleColor还没有设置。

改变uniform的值与设置vertex attribute类似，首先你要获取uniform的位置。

	GLint uniColor = glGetUniformLocation(shaderProgram, "triangleColor");

然后使用glUnifomXY函数来设置uniform，X表数目，Y表类型。

	glUniform3f(uniColor, 1.0f, 0.0f, 0.0f);

如果你现在运行程序，你会发现三角形变成红色的了。下面代码会使颜色随时间渐变：

	auto t_start = std::chrono::high_resolution_clock::now();

	...

	auto t_now = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

	glUniform3f(uniColor, (sin(time * 4.0f) + 1.0f) / 2.0f, 0.0f, 0.0f);

虽然这个例子可能十分有趣，它演示了uniform可以在运行时控制shader的行为。而vertex attribute却是十分适合用来表达各个vertex的。

<div class="livedemo_wrap">
	<div class="livedemo" id="demo_c2_uniforms" style="background: url('../../media/img/c2_window3.png')">
		<canvas width="640" height="480"></canvas>
		<script type="text/javascript" src="/content/demos/c2_uniforms.js"></script>
	</div>
</div>

看 [the code](/content/code/c2_triangle_uniform.txt)这个代码，如果你对上面的实现有问题。 

增加更多的颜色
========

虽然uniform有他们的用处，对于顶点的颜色，我们还是需要对不同的顶点设置各自的颜色。现在我们增加一个color attribute来完成这个任务。

首先我们要增加额外的数据到顶点数组中。因为我们这里不考虑alpha通道的情况，所以我们只添加了R/G/B三个通道的数据：

	float vertices[] = {
		 0.0f,  0.5f, 1.0f, 0.0f, 0.0f, // Vertex 1: Red
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Vertex 2: Green
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f  // Vertex 3: Blue
	};

接下来，我们改变顶点着色器，使它能够取得这些颜色，同时传递给片段着色器：

	#version 150

	in vec2 position;
	in vec3 color;

	out vec3 Color;

	void main()
	{
		Color = color;
		gl_Position = vec4(position, 0.0, 1.0);
	}

在片段着色器中加入Color输入变量：

	#version 150

	in vec3 Color;

	out vec4 outColor;

	void main()
	{
		outColor = vec4(Color, 1.0);
	}

这里要确保顶点着色器输出地颜色变量和片段着色器接受的颜色变量的变量名一致，否则两个shader不能够顺利地链接。

现在，我们更新attribute与VBO的关联：

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE,
						   5*sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE,
						   5*sizeof(float), (void*)(2*sizeof(float)));

第五个变量现在被设置为`5*sizeof（float）`因为我们每个顶点的数据长度变化了。并且颜色attribute的offset要设置成`2*sizeof（float）`;

到此，我们完成了。

<img src="../../media/img/c2_window2.png" alt="" />

你现在应该能够理解vertex attribute和shader了。如果你遇到问题，你可以在评论中提问或者看下源码 [source code](/content/code/c2_color_triangle.txt).

Element buffers
========

至此，所有顶点都是按他们即将绘制的顺序排列的。如果你要增加一个新的三角形，你就得再添加3个顶点到顶点数组内。但实际上如果2个三角形有一条共享的边，那么其中的2对顶点就会是重复的，这将会增加顶点数组的大小。有一个办法能够控制绘制时使用顶点的顺序，通过它我们还可以减少顶点的数量。这会减少很多的内存开销，因为在实际的模型中，一个vertex往往被3个不同的三角形公用。

一个element array里面是一些列无符号整数，这些无符号整数索引到当前活动的VBO中的顶点。如果我们要像之前那样绘制三角形，那么这个element array应该是这样的：

	GLuint elements[] = {
		0, 1, 2
	};

它们通过一个EBO来传入到显卡中：

	GLuint ebo;
	glGenBuffers(1, &ebo);

	...

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(elements), elements, GL_STATIC_DRAW);

唯一不同的是，我们现在使用一个枚举为`GL_ELEMENT_ARRAY_BUFFER`。

要使这个功能生效，我们要换一个绘制函数：

	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

第一个参数与glDrawArrays相同，其它的都与现在的EBO对应了。第二个参数表示需要绘制的顶点数量。第三个参数表示EBO中数据的类型。最后一个参数表示偏移量。唯一的区别是，我们现在谈论的是索引，而不是顶点。

为了演示EBO的作用，我们现在尝试绘制2个三角形。我们先不用EBO，那么顶点数组应该是这样的：

	float vertices[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right

		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, // Bottom-left
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f  // Top-left
	};

现在我们使用glDrawArrays来绘制，那么EBO将会被忽略：

	glDrawArrays(GL_TRIANGLES, 0, 6);

当然，两个三角形都能按预期的绘制。下面使用EBO来绘制时，会减少顶点数据：

	float vertices[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // Bottom-left
	};

	...

	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};

	...

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

EBO中依旧设置了6个顶点，但如你所见其中2个顶点是重复使用的。这减少了顶点数据。

<img src="../../media/img/c2_window4.png" alt="" />

如果你遇到问题，请看完整地代码[source code](/content/code/c2_triangle_elements.txt).

这章涵盖了使用OpenGL来绘制的所有基本内容。你应该充分了解本届内容后，再学习下面的内容。所以我建议你先做下下面的习题。

Exercises
========

- 修改顶点着色器，使三角形上下颠倒. ([Solution](/content/code/c2_exercise_1.txt))
- 修改片段着色器，反转色彩。. ([Solution](/content/code/c2_exercise_2.txt))
- 修改程序，使得顶点数组内颜色仅有一个float，它将表示灰度值. ([Solution](/content/code/c2_exercise_3.txt))
