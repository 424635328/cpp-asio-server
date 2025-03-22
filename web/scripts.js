document.addEventListener("DOMContentLoaded", function () {
  const numThreads = navigator.hardwareConcurrency || 4;
  const threadCountElement = document.getElementById("thread-count");
  const wasmButton = document.getElementById("wasmButton");
  const dynamicContent = document.getElementById("dynamicContent");
  const imageInput = document.getElementById("imageInput");
  const wasmModulePath = "/web/module.wasm";

  let imageWidth = 200;
  let imageHeight = 150;
  let wasmInstance = null;
  let rgbArray = null; // 保存图像数据

  if (threadCountElement) {
    threadCountElement.innerText = numThreads;
    console.log("线程数已成功写入到 threadCountElement");
  } else {
    console.warn("未找到 threadCountElement 元素，无法显示线程数");
  }

  //  模拟 WASI 环境 (简单的 polyfill)
  const wasi = {
    args: [],
    env: {},
    fd_write: function (fd, iovs_ptr, iovs_len, nwritten_ptr) {
      // 简单的 stdout 模拟
      let memory = wasmInstance.exports.memory; // 获取内存
      let view = new DataView(memory.buffer);

      let str = "";
      for (let i = 0; i < iovs_len; i++) {
        let base = view.getInt32(iovs_ptr + i * 8, true); // iovs[i].buf
        let len = view.getInt32(iovs_ptr + i * 8 + 4, true);  // iovs[i].buf_len
        str += new TextDecoder().decode(new Uint8Array(memory.buffer, base, len));
      }

      console.log("WASI Output:", str); // 输出到控制台
      return 0; // 成功
    },
    proc_exit: function(code) {
        console.log("WASM program exited with code:", code);
    },
    // 示例：添加 clock_time_get (根据你的 WASM 模块的需求添加)
    clock_time_get: function(id, precision, out_ptr) {
        // 模拟一个时间
        const now = BigInt(Date.now());
        let memory = wasmInstance.exports.memory;
        let view = new DataView(memory.buffer);
        view.setBigInt64(out_ptr, now, true); // Little-endian
        return 0;
    },
    random_get: function(buf_ptr, buf_len) {
        let memory = wasmInstance.exports.memory;
        let buffer = new Uint8Array(memory.buffer, buf_ptr, buf_len);
        for (let i = 0; i < buf_len; i++) {
            buffer[i] = Math.floor(Math.random() * 256); // 0-255
        }
        return 0;
    }
  };

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
      console.log("WASM buffer 获取成功，buffer大小:", buffer.byteLength);

      // 创建 importObject，包含 WASI 导入
      const importObject = {
        wasi_snapshot_preview1: wasi, //  关键: 添加 WASI
        env: {
          // 添加 emscripten_notify_memory_growth 函数
          emscripten_notify_memory_growth: function() {
            //  这个函数可以为空，但必须存在且可调用
            console.log("emscripten_notify_memory_growth called"); // 可以添加日志
          },

          // 示例：添加 JavaScript 函数导入
          my_js_function: function (arg) {
            console.log("JavaScript function called with:", arg);
          },
          // 添加 console.log 函数的导入 (如果你的 WASM 需要 console.log)
          console_log: function(arg) {
            console.log("WASM console.log:", arg);
          }
        },
      };

      console.log("importObject 创建成功:", importObject);

      try {
        const result = await WebAssembly.instantiate(buffer, importObject);
        console.log("WebAssembly.instantiate 成功:", result);

        const instance = result.instance;
        wasmInstance = instance;

        console.log("WASM 模块实例化完成");
        console.log("导出的函数:", instance.exports);

        return instance;
      } catch (instantiateError) {
        console.error("WebAssembly.instantiate 失败:", instantiateError);
        dynamicContent.innerText = "WASM 模块加载失败: " + instantiateError.message; // 显示在页面上
        throw instantiateError;
      }
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
          const rgbCanvas = document.createElement("canvas");
          rgbCanvas.width = imageWidth;
          rgbCanvas.height = imageHeight;
          const rgbCtx = rgbCanvas.getContext("2d");
          rgbCtx.drawImage(img, 0, 0);
          const imageData = rgbCtx.getImageData(0, 0, imageWidth, imageHeight);
          rgbArray = new Uint8Array(imageData.data.buffer);  // 保存图像数据
          resolve(rgbArray);  // 返回 Uint8Array
        };
        img.onerror = () => {
           dynamicContent.innerText = "图片加载失败，请检查图片是否损坏或格式是否正确。"; // 显示在页面上
           reject();
        }
        img.src = e.target.result;
      };
      reader.onerror = () => {
         dynamicContent.innerText = "无法读取图片文件。";  // 显示在页面上
         reject();
      }
      reader.readAsDataURL(file);
    });
  }

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
        dynamicContent.innerText = "请先选择图片！";
        throw new Error("请先选择图片！");
      }

      if (!instance) {
         dynamicContent.innerText = "WASM 实例未加载！";
        throw new Error("WASM 实例未加载！");
      }

      console.log("convertToGrayscale: instance =", instance);
      console.log("convertToGrayscale: instance.exports =", instance.exports);

      const memory = instance.exports.memory;
      const gray_scale = instance.exports.gray_scale;
      const allocate_memory = instance.exports.allocate_memory;
      const free_memory = instance.exports.free_memory;
      const width = imageWidth;
      const height = imageHeight;

       console.log("convertToGrayscale: memory =", memory);
       console.log("convertToGrayscale: gray_scale =", gray_scale);
       console.log("convertToGrayscale: allocate_memory =", allocate_memory);
       console.log("convertToGrayscale: free_memory =", free_memory);

      if (
        typeof allocate_memory !== "function" ||
        typeof free_memory !== "function" ||
        typeof gray_scale !== "function" ||
        !memory
      ) {
        dynamicContent.innerText = "WASM 模块缺少必要的导出函数或内存。请检查 WASM 模块是否正确编译。";
        throw new Error(
          "WASM 模块缺少必要的导出函数或内存。请检查 WASM 模块是否正确编译。"
        );
      }

      const rgbDataByteSize = rgbArray.length * rgbArray.BYTES_PER_ELEMENT;
      const grayDataByteSize =
        width * height * Uint8ClampedArray.BYTES_PER_ELEMENT;

      let rgbDataPtr, grayDataPtr;
      try {
        rgbDataPtr = allocate_memory(rgbDataByteSize);
        grayDataPtr = allocate_memory(grayDataByteSize);
      } catch (e) {
        dynamicContent.innerText = "WASM 内存分配失败：" + e.message; // 显示在页面上
        throw new Error("WASM 内存分配失败：" + e.message);
      }

      const rgbDataHeap = new Uint8Array(
        memory.buffer,
        rgbDataPtr,
        rgbArray.length
      );
      rgbDataHeap.set(rgbArray);

      try {
        gray_scale(rgbDataPtr, width, height, grayDataPtr);
      } catch (e) {
        dynamicContent.innerText = "灰度转换函数执行失败：" + e.message; // 显示在页面上
        throw new Error("灰度转换函数执行失败：" + e.message);
      }

      const grayResult = new Uint8ClampedArray(
        memory.buffer,
        grayDataPtr,
        width * height
      );

      const outputWidth = 200;
      const outputHeight = 150;

      const grayCanvas = document.createElement("canvas");
      grayCanvas.width = outputWidth;
      grayCanvas.height = outputHeight;
      const grayCtx = grayCanvas.getContext("2d");

      const grayImageData = createGrayscaleImageData(grayResult, width, height);

      // 创建 ImageBitmap 并绘制
      createImageBitmap(grayImageData).then((bitmap) => {
          grayCtx.drawImage(bitmap, 0, 0, outputWidth, outputHeight);

          // 创建可点击放大的图像包装器
          const imageWrapper = document.createElement("div");
          imageWrapper.style.position = "relative"; // 允许定位放大镜
          imageWrapper.style.display = "inline-block"; // 使尺寸适应内容
          imageWrapper.style.cursor = "zoom-in"; // 鼠标悬停时的光标

          // 添加放大的样式（你需要在 CSS 中定义这些）
          grayCanvas.classList.add("grayscale-image");

          // 将 canvas 添加到包装器
          imageWrapper.appendChild(grayCanvas);

          // 移除之前的图像（如果存在）
          dynamicContent.innerHTML = "";
          dynamicContent.appendChild(imageWrapper);

          // 点击时放大/缩小
          imageWrapper.addEventListener("click", () => {
              console.log("Image wrapper clicked!"); // 确保事件被触发
              if (imageWrapper.classList.contains("zoomed")) {
                  imageWrapper.classList.remove("zoomed");
                  imageWrapper.style.cursor = "zoom-in";
              } else {
                  imageWrapper.classList.add("zoomed");
                  imageWrapper.style.cursor = "zoom-out";
              }
          });
      });
      try {
        free_memory(rgbDataPtr, rgbDataByteSize);
        free_memory(grayDataPtr, grayDataByteSize);
      } catch (e) {
        console.warn("WASM 内存释放失败：" + e.message);
      }
    } catch (error) {
      console.error("WASM 执行过程中发生错误:", error);
      // 确保错误信息显示在页面上
      dynamicContent.innerText = error.message;
    }
  }

  if (imageInput) {
    imageInput.addEventListener("change", async function () {
      const file = imageInput.files[0];
      if (file) {
        try {
          dynamicContent.innerText = "正在处理图片..."; // 提示用户正在处理
           await processImage(file); // 处理图片. rgbArray 会被赋值
          console.log("图像处理成功");
          dynamicContent.innerText = ""; // 清空"正在处理图片..."的提示

          // 启用按钮
          if (wasmButton) {
              wasmButton.disabled = false;
          }

          if (wasmButton && dynamicContent) {
             // 移除之前的 onclick 事件监听器
             wasmButton.onclick = null;

             wasmButton.onclick = async () => {
                console.log("WASM 按钮被点击");

                if (!rgbArray) {  //  如果没有图像数据
                    dynamicContent.innerText = "请先选择一张图片！";
                    return;
                }

                try {
                    if (!wasmInstance) {
                        wasmInstance = await loadWasmModule(wasmModulePath);
                        console.log("WASM 模块加载和实例化成功");
                    }

                    await convertToGrayscale(wasmInstance, rgbArray);

                } catch (error) {
                    console.error("WASM 执行过程中发生错误:", error);
                    dynamicContent.innerText = error.message;
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
          rgbArray = null;  // 重置图像数据

          // 禁用按钮
          if (wasmButton) {
              wasmButton.disabled = true;
          }
        }
      } else {
        rgbArray = null;  // 如果没有选择文件，重置图像数据
        dynamicContent.innerText = "请选择一张图片。";
        // 禁用按钮
        if (wasmButton) {
            wasmButton.disabled = true;
        }

      }
    });
  } else {
    console.warn("未找到 imageInput 元素");
  }

    //  初始状态禁用按钮，直到选择了图像
    if (wasmButton) {
        wasmButton.disabled = true; // 初始状态禁用

        // 添加监听器，根据 input 是否有文件来启用/禁用按钮
        imageInput.addEventListener("change", function() {
            wasmButton.disabled = !imageInput.files.length; // 检查是否有选择文件
        });
    }
});