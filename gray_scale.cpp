#include <stdlib.h>
#include <stdint.h>
#include <emscripten.h>
#include <assert.h>
// 将 RGB 像素数据转换为灰度数据
// 参数：
//   rgb_data: 指向 RGB 像素数据数组的指针 (uint8_t 类型，每像素 3 字节：R, G, B)
//   width: 图像宽度
//   height: 图像高度
//   gray_data: 指向灰度像素数据数组的指针 (uint8_t 类型，每像素 1 字节)
//
//  灰度值计算公式: Gray = 0.299 * R + 0.587 * G + 0.114 * B

extern "C"
{
  EMSCRIPTEN_KEEPALIVE
  int gray_scale(uint8_t *rgb_data, int width, int height, uint8_t *gray_data)
  {
    try
    {
      int image_size = width * height;
      for (int i = 0; i < image_size; i++)
      {
        assert(i * 3 + 0 < width * height * 3);
        assert(i * 3 + 1 < width * height * 3);
        assert(i * 3 + 2 < width * height * 3);
        uint8_t r = rgb_data[i * 3 + 0];
        uint8_t g = rgb_data[i * 3 + 1];
        uint8_t b = rgb_data[i * 3 + 2];

        // 计算灰度值 (使用浮点数，然后转换为整数)
        float gray_f = 0.299f * r + 0.587f * g + 0.114f * b;
        uint8_t gray = (uint8_t)gray_f; // 截断，效果更好

        assert(i < width * height);
        gray_data[i] = gray;
      }
      return 0; // 成功
    }
    catch (...)
    {
      return -1; // 失败
    }
  }

  EMSCRIPTEN_KEEPALIVE
  uintptr_t allocate_memory(size_t size)
  {
    void *ptr = malloc(size);
    return reinterpret_cast<uintptr_t>(ptr);
  }

  EMSCRIPTEN_KEEPALIVE
  void free_memory(uintptr_t ptr, size_t size)
  {
    void *real_ptr = reinterpret_cast<void *>(ptr);
    free(real_ptr);
  }
}

int main()
{
  return 0;
}