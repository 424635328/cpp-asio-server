document.addEventListener("DOMContentLoaded", function () {
  const numThreads = navigator.hardwareConcurrency || 4;
  const threadCountElement = document.getElementById("thread-count");
  const wasmButton = document.getElementById("wasmButton");
  const dynamicContent = document.getElementById("dynamicContent");
  const imageInput = document.getElementById("imageInput");
  const wasmModulePath = "/web/module.wasm";

  let imageWidth = 200;
  let imageHeight = 150;

  if (threadCountElement) {
    threadCountElement.innerText = numThreads;
    console.log("线程数已成功写入到 threadCountElement");
  } else {
    console.warn("未找到 threadCountElement 元素，无法显示线程数");
  }

  async function loadWasmModule(modulePath) {
    dynamicContent.innerText = "正在加载 WASM 模块...";
    try {
      console.log("开始加载 WASM 模块:", modulePath);
      const response = await fetch(modulePath);
      console.log("WASM 模块加载状态:", response.status);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const buffer = await response.arrayBuffer();
      const result = await WebAssembly.instantiate(buffer);

      const instance = result.instance;

      console.log("WASM 模块实例化完成");
      console.log("导出的函数:", instance.exports);

      return instance;
    } catch (error) {
      console.error("加载 WASM 模块时发生错误:", error);
      let errorMessage = "错误：无法加载 WASM 模块。\n" + error.message;
      if (error.stack) {
        errorMessage += "\n" + error.stack;
      }
      dynamicContent.innerText = errorMessage;
      throw error;
    }
  }

  function processImage(file) {
    return new Promise((resolve, reject) => {
      const reader = new FileReader();
      reader.onload = function (e) {
        const img = new Image();
        img.onload = function () {
          imageWidth = img.width;
          imageHeight = img.height;
          //优化：直接从图片获取像素数据，无需创建额外的 canvas
          const rgbCanvas = document.createElement("canvas");
          rgbCanvas.width = imageWidth;
          rgbCanvas.height = imageHeight;
          const rgbCtx = rgbCanvas.getContext("2d");
          rgbCtx.drawImage(img, 0, 0);
          const imageData = rgbCtx.getImageData(0, 0, imageWidth, imageHeight);
          resolve(new Uint8Array(imageData.data.buffer));
        };
        img.onerror = reject;
        img.src = e.target.result;
      };
      reader.onerror = reject;
      reader.readAsDataURL(file);
    });
  }
    // 优化：将灰度转换的逻辑封装成单独的函数，方便复用和测试
    function createGrayscaleImageData(grayResult, width, height) {
        const grayImageData = new ImageData(width, height);
        const data = grayImageData.data;
        for (let i = 0; i < width * height; i++) {
            const grayValue = grayResult[i];
            data[i * 4] = grayValue;
            data[i * 4 + 1] = grayValue;
            data[i * 4 + 2] = grayValue;
            data[i * 4 + 3] = 255;
        }
        return grayImageData;
    }

  async function convertToGrayscale(instance, rgbArray) {
    try {
      if (!rgbArray) {
        throw new Error("请先选择图片！");
      }

      if (!instance) {
        throw new Error("WASM 实例未加载！");
      }

      // 1.  从 WASM 实例中获取导出的函数和内存
      const memory = instance.exports.a; // memory
      const gray_scale = instance.exports.c; // gray_scale
      const allocate_memory = instance.exports.b; // allocate_memory
      const free_memory = instance.exports.d; // free_memory
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
      const outputWidth = 200; // 你希望的输出宽度
      const outputHeight = 150; // 你希望的输出高度

      const grayCanvas = document.createElement("canvas");
      grayCanvas.width = outputWidth; // 使用输出宽度
      grayCanvas.height = outputHeight; // 使用输出高度
      const grayCtx = grayCanvas.getContext("2d");

        //    直接使用灰度数据创建 ImageData 对象
        const grayImageData = createGrayscaleImageData(grayResult, width, height);
        //    使用createImageBitmap进行缩放，性能更好
        createImageBitmap(grayImageData).then(bitmap => {
            grayCtx.drawImage(bitmap, 0, 0, outputWidth, outputHeight);
            dynamicContent.innerHTML = "";
            dynamicContent.appendChild(grayCanvas);
        });

      // 8. 释放 WASM 内存
      free_memory(rgbDataPtr, rgbDataByteSize);
      free_memory(grayDataPtr, grayDataByteSize);
    } catch (error) {
      console.error("WASM 执行过程中发生错误:", error);
      dynamicContent.innerText = error.message;
    }
  }

  if (imageInput) {
    imageInput.addEventListener("change", async function () {
      const file = imageInput.files[0];
      if (file) {
        try {
          // 修改：将 processImage 的返回值直接赋值给 rgbArray
          const rgbArray = await processImage(file);
          console.log("图像处理成功");
          // 修改：将 rgbArray 传递给 convertToGrayscale 函数
          if (wasmButton && dynamicContent) {
            wasmButton.addEventListener("click", async function () {
              console.log("WASM 按钮被点击");
              try {
                const instance = await loadWasmModule(wasmModulePath);
                console.log("WASM 模块加载和实例化成功");

                await convertToGrayscale(instance, rgbArray);
              } catch (error) {
                console.error("WASM 执行过程中发生错误:", error);
              }
            });
          } else {
            console.warn(
              "未找到 wasmButton 或 dynamicContent 元素，WASM 功能将无法正常工作"
            );
          }
        } catch (error) {
          console.error("图像处理失败:", error);
          dynamicContent.innerText = "图像处理失败: " + error.message;
        }
      }
    });
  } else {
    console.warn("未找到 imageInput 元素");
  }
    if (imageInput) {
        imageInput.addEventListener("change", async function () {
            const file = imageInput.files[0];
            if (file) {
                try {
                    // 修改：将 processImage 的返回值直接赋值给 rgbArray
                    const rgbArray = await processImage(file);
                    console.log("图像处理成功");
                    // 修改：将 rgbArray 传递给 convertToGrayscale 函数
                    if (wasmButton && dynamicContent) {
                        wasmButton.onclick = async () => { //使用 onclick 避免多次绑定
                            console.log("WASM 按钮被点击");
                            try {
                                const instance = await loadWasmModule(wasmModulePath);
                                console.log("WASM 模块加载和实例化成功");

                                await convertToGrayscale(instance, rgbArray);
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
                    dynamicContent.innerText = "图像处理失败: " + error.message;
                }
            }
        });
    } else {
        console.warn("未找到 imageInput 元素");
    }
});