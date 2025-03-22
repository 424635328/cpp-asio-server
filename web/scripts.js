document.addEventListener("DOMContentLoaded", function () {
  const numThreads = navigator.hardwareConcurrency || 4; // 获取硬件线程数，默认为4
  const threadCountElement = document.getElementById("thread-count"); // 获取线程数显示元素
  const wasmButton = document.getElementById("wasmButton"); // 获取wasm按钮
  const dynamicContent = document.getElementById("dynamicContent"); // 获取动态内容显示元素
  const imageInput = document.getElementById("imageInput"); // 获取图片输入元素
  const wasmModulePath = "/web/module.wasm"; // wasm模块路径

  let imageWidth = 200; // 默认图片宽度
  let imageHeight = 150; // 默认图片高度

  if (threadCountElement) {
    threadCountElement.innerText = numThreads; // 设置线程数
    console.log("线程数已成功写入到 threadCountElement");
  } else {
    console.warn("未找到 threadCountElement 元素，无法显示线程数");
  }

  async function loadWasmModule(modulePath) {
    dynamicContent.innerText = "正在加载 WASM 模块..."; // 显示加载信息
    try {
      console.log("开始加载 WASM 模块:", modulePath);
      const response = await fetch(modulePath); // 获取wasm模块
      console.log("WASM 模块加载状态:", response.status);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`); // 抛出错误
      }

      const buffer = await response.arrayBuffer(); // 获取buffer
      const result = await WebAssembly.instantiate(buffer); // 实例化wasm模块

      const instance = result.instance; // 获取实例

      console.log("WASM 模块实例化完成");
      console.log("导出的函数:", instance.exports);

      return instance; // 返回实例
    } catch (error) {
      console.error("加载 WASM 模块时发生错误:", error);
      let errorMessage = "错误：无法加载 WASM 模块。\n" + error.message; // 错误信息
      if (error.stack) {
        errorMessage += "\n" + error.stack; // 添加堆栈信息
      }
      dynamicContent.innerText = errorMessage; // 显示错误信息
      throw error;
    }
  }

  function processImage(file) {
    return new Promise((resolve, reject) => {
      const reader = new FileReader(); // 文件读取
      reader.onload = function (e) {
        const img = new Image(); // 创建图片对象
        img.onload = function () {
          imageWidth = img.width; // 获取图片宽度
          imageHeight = img.height; // 获取图片高度
          const rgbCanvas = document.createElement("canvas"); // 创建canvas
          rgbCanvas.width = imageWidth; // 设置canvas宽度
          rgbCanvas.height = imageHeight; // 设置canvas高度
          const rgbCtx = rgbCanvas.getContext("2d"); // 获取context
          rgbCtx.drawImage(img, 0, 0); // 绘制图片
          const imageData = rgbCtx.getImageData(0, 0, imageWidth, imageHeight); // 获取imageData
          resolve(new Uint8Array(imageData.data.buffer)); // 返回Uint8Array
        };
        img.onerror = reject; // 错误处理
        img.src = e.target.result; // 设置图片src
      };
      reader.onerror = reject; // 错误处理
      reader.readAsDataURL(file); // 读取文件
    });
  }
    function createGrayscaleImageData(grayResult, width, height) {
        const grayImageData = new ImageData(width, height); // 创建ImageData
        const data = grayImageData.data; // 获取data
        for (let i = 0; i < width * height; i++) { // 遍历像素
            const grayValue = grayResult[i]; // 获取灰度值
            data[i * 4] = grayValue; // 设置R
            data[i * 4 + 1] = grayValue; // 设置G
            data[i * 4 + 2] = grayValue; // 设置B
            data[i * 4 + 3] = 255; // 设置A
        }
        return grayImageData; // 返回ImageData
    }

  async function convertToGrayscale(instance, rgbArray) {
    try {
      if (!rgbArray) {
        throw new Error("请先选择图片！"); // 抛出错误
      }

      if (!instance) {
        throw new Error("WASM 实例未加载！"); // 抛出错误
      }

      // 1.  从 WASM 实例中获取导出的函数和内存
      const memory = instance.exports.a; 
      const gray_scale = instance.exports.c; // 获取灰度函数
      const allocate_memory = instance.exports.b; // 分配内存函数
      const free_memory = instance.exports.d; // 释放内存函数
      const width = imageWidth;
      const height = imageHeight;

      // 2. 计算需要的内存大小
      const rgbDataByteSize = rgbArray.length * rgbArray.BYTES_PER_ELEMENT;
      const grayDataByteSize =
        width * height * Uint8ClampedArray.BYTES_PER_ELEMENT;

      // 3. 在 WASM 内存中分配空间
      const rgbDataPtr = allocate_memory(rgbDataByteSize);
      const grayDataPtr = allocate_memory(grayDataByteSize);

      // 4. 将 RGB 数据写入 WASM 内存
      const rgbDataHeap = new Uint8Array(
        memory.buffer,
        rgbDataPtr,
        rgbArray.length
      );
      rgbDataHeap.set(rgbArray);

      // 5. 调用 WASM 函数
      gray_scale(rgbDataPtr, width, height, grayDataPtr);

      // 6. 从 WASM 内存读取灰度数据
      const grayResult = new Uint8ClampedArray(
        memory.buffer,
        grayDataPtr,
        width * height
      );

      // 7. 创建灰度图像并显示
      const outputWidth = 200; // 输出宽度
      const outputHeight = 150; //输出高度

      const grayCanvas = document.createElement("canvas"); // 创建canvas
      grayCanvas.width = outputWidth; // 设置canvas宽度
      grayCanvas.height = outputHeight; // 设置canvas高度
      const grayCtx = grayCanvas.getContext("2d"); // 获取context

        const grayImageData = createGrayscaleImageData(grayResult, width, height);
        //    使用createImageBitmap进行缩放，性能更好
        createImageBitmap(grayImageData).then(bitmap => {
            grayCtx.drawImage(bitmap, 0, 0, outputWidth, outputHeight); // 绘制灰度图片
            dynamicContent.innerHTML = ""; // 清空内容
            dynamicContent.appendChild(grayCanvas); // 添加canvas
        });

      // 8. 释放 WASM 内存
      free_memory(rgbDataPtr, rgbDataByteSize);
      free_memory(grayDataPtr, grayDataByteSize);
    } catch (error) {
      console.error("WASM 执行过程中发生错误:", error);
      dynamicContent.innerText = error.message; // 显示错误信息
    }
  }
  if (imageInput) {
    imageInput.addEventListener("change", async function () {
      const file = imageInput.files[0]; // 获取文件
      if (file) {
        try {
          const rgbArray = await processImage(file); // 处理图片
          console.log("图像处理成功");
          if (wasmButton && dynamicContent) {
            wasmButton.onclick = async () => { //使用 onclick 避免多次绑定
              console.log("WASM 按钮被点击");
              try {
                const instance = await loadWasmModule(wasmModulePath); // 加载wasm模块
                console.log("WASM 模块加载和实例化成功");

                await convertToGrayscale(instance, rgbArray); // 转换成灰度图
              } catch (error) {
                console.error("WASM 执行过程中发生错误:", error);
              }
            };
          } else {
            console.warn(
              "未找到 wasmButton 或 dynamicContent 元素，WASM 功能将无法正常工作"
            );
          }
        } catch (error) {
          console.error("图像处理失败:", error);
          dynamicContent.innerText = "图像处理失败: " + error.message; // 显示错误信息
        }
      }
    });
  } else {
    console.warn("未找到 imageInput 元素");
  }
});