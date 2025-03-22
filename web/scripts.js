document.addEventListener('DOMContentLoaded', function() {
  // Detect the number of threads
  const numThreads = navigator.hardwareConcurrency || 4;
  const threadCountElement = document.getElementById('thread-count'); // 获取元素

  console.log("检测到的线程数:", numThreads); // 添加日志

  if (threadCountElement) { // 检查元素是否存在
    threadCountElement.innerText = numThreads;
    console.log("线程数已成功写入到 threadCountElement"); // 添加日志
  } else {
    console.warn("未找到 threadCountElement 元素，无法显示线程数"); // 添加日志
  }

  const wasmButton = document.getElementById('wasm-button');
  const dynamicContent = document.getElementById('dynamic-content');

  if (wasmButton && dynamicContent) {
    wasmButton.addEventListener('click', function() {
      console.log("WASM 按钮被点击"); // 添加日志
      loadWasmModule('./module.wasm')
        .then(instance => {
          console.log("WASM 模块加载和实例化成功"); // 添加日志
          // Example: Calling a function named 'calculate' that takes two integers and returns an integer
          const result = instance.exports.calculate(10, 20); // Replace with your WASM function and arguments
          console.log("WASM 函数调用结果:", result); // 添加日志
          dynamicContent.innerText = "WASM 结果: " + result;
        })
        .catch(error => {
          console.error("加载 WASM 模块时发生错误:", error);
          dynamicContent.innerText = "错误：无法加载 WASM 模块。";
        });
    });
  } else {
    console.warn("未找到 wasmButton 或 dynamicContent 元素，WASM 功能将无法正常工作"); // 添加日志
  }

  async function loadWasmModule(modulePath) {
    try {
      console.log("开始加载 WASM 模块:", modulePath); // 添加日志
      const response = await fetch(modulePath);
      console.log("WASM 模块加载状态:", response.status); // 添加日志
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const buffer = await response.arrayBuffer();
      console.log("WASM 模块加载完成，开始编译"); // 添加日志
      const module = await WebAssembly.compile(buffer);
      console.log("WASM 模块编译完成，开始实例化"); // 添加日志

      // 传递一个空对象作为 imports 参数
      const importObject = {};
      const instance = await WebAssembly.instantiate(module, importObject);

      console.log("WASM 模块实例化完成"); // 添加日志
      return instance;
    } catch (error) {
      console.error("加载 WASM 模块时发生错误:", error);
      dynamicContent.innerText = "错误：无法加载 WASM 模块。";
      throw error; // Re-throw the error so that the caller can handle it
    }
  }
});