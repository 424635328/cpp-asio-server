// script.js

async function loadWasm() {
    try {
      // 1. Load the WASM file
      const response = await fetch('module.wasm');
  
      if (!response.ok) {
        throw new Error(`Failed to load WASM module: ${response.status} - ${response.statusText}`);
      }
  
      const buffer = await response.arrayBuffer();
      const module = await WebAssembly.compile(buffer);
  
      // 2. Instantiate the WASM module
      const instance = await WebAssembly.instantiate(module); // No imports for this example
  
      // 3. Access WASM exports
      const wasmAdd = instance.exports.add; // 假设 WASM 导出了一个 add 函数
  
      if (!wasmAdd) {
        throw new Error("The WASM module doesn't export the 'add' function.");
      }
  
       //4. Update Content
      const result = wasmAdd(5, 3); // 调用 WASM 函数
      document.getElementById('dynamic-content').textContent = `The result from WebAssembly is: ${result}`;
  
       //5. Add event listener to button
      document.getElementById('wasm-button').addEventListener('click', () => {
        const anotherResult = wasmAdd(10, 20);
        alert(`Another result from WebAssembly: ${anotherResult}`);
      });
  
      console.log('WebAssembly module loaded and initialized.');
  
    } catch (error) {
      console.error('Error loading and initializing WebAssembly module:', error);
    }
  }
  
  loadWasm(); // Call the function to load and initialize the WASM module