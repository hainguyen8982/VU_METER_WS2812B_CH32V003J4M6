# Walkthrough - Stereo VU LED Meter 32 LEDs (CH32V003J4M6)

Đã hoàn thành toàn bộ mã nguồn C và tài liệu hướng dẫn đấu nối cho dự án **VU LED Meter 2 Kênh Stereo (32 LED WS2812)** sử dụng vi điều khiển **CH32V003J4M6** (gói SOP-8).

---

## Các thành phần đã triển khai

### 1. Driver điều khiển 32 LED WS2812 (`PC2`)
- **[ws2812.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.h)** & **[ws2812.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.c)**
- Xuất tín hiệu bit-banging trực tiếp trên xung 48MHz (T0H/T1H chính xác đến từng nanosecond).
- Hỗ trợ quản lý đệm 32 LED, bảng màu RGB, hiệu ứng vòng màu Rainbow Wheel, và điều chỉnh độ sáng chung (Master Brightness Scaling).

### 2. Mạch đọc Audio ADC Dual Channel (`PA2` & `PD6`)
- **[adc_audio.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.h)** & **[adc_audio.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.c)**
- Đọc đồng thời 2 kênh âm thanh độc lập: Kênh Trái (`PA2` / ADC Ch0) và Kênh Phải (`PD6` / ADC Ch6).
- Tích hợp bộ lọc chống nhiễu nền (Noise Gate) và bộ tự động điều khiển độ lợi (Auto Gain Control - AGC) tự thích ứng với âm lượng nguồn nhạc.
- **Lưu ý:** Chuyển hướng UART TX mặc định trong [debug.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/Debug/debug.h) sang chân `PD5` (không có chân vật lý trên SOP-8) để tránh xung đột phần cứng khi đưa tín hiệu âm thanh vào chân 1 (`PD6`). Lưu ý chân `PA2` dùng chung làm ngõ vào âm thanh Kênh Trái và chân nạp `SWCLK`.

### 3. Driver Nút bấm chuyển chế độ (`PC4`)
- **[button.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.h)** & **[button.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.c)**
- Quét nút bấm non-blocking với cơ chế chống dội (Debounce).
- Nhấn ngắn: Chuyển lần lượt qua **6 hiệu ứng**.
- Nhấn giữ (>0.8s): Thay đổi **4 mức độ sáng** LED.

### 4. Engine Hiệu ứng ánh sáng & Đỉnh rơi Peak Hold
- **[vu_effects.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.h)** & **[vu_effects.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.c)**
- Tích hợp 6 chế độ ánh sáng độc đáo (Classic Green-Yellow-Red, Rainbow Spectrum, Center-Out Symmetric, Fire Heatmap, Ocean Blue, Solid Purple/Cyan).
- Mô phỏng vật lý đỉnh rơi giọt nước (Peak Hold duration & Gravity Decay) cho trải nghiệm nhìn mượt mà chuyên nghiệp.

### 5. Khởi tạo Hệ thống & Vòng lặp chính
- **[main.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/main.c)**
- Khởi động hệ thống 48MHz, chạy hiệu ứng Boot Cầu vồng lướt 32 LED và duy trì tốc độ làm tươi **60 - 80 FPS**.

### 6. Sơ đồ đấu nối & Tài liệu hướng dẫn
- **[Huong_Dan_Su_Dung.md](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/Huong_Dan_Su_Dung.md)**
- Hướng dẫn chi tiết sơ đồ chân SOP-8, sơ đồ ghép nối chuỗi 4 module 1x8 WS2812 và mạch chia điện áp DC Bias cho cổng vào Audio ADC.
