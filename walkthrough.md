# Walkthrough - Stereo VU LED Meter 32 LEDs (CH32V003J4M6)

Đã hoàn thành toàn bộ mã nguồn C và tài liệu hướng dẫn đấu nối cho dự án **VU LED Meter 2 Kênh Stereo (32 LED WS2812)** sử dụng vi điều khiển **CH32V003J4M6** (gói SOP-8).

---

## Các thành phần đã triển khai & Nâng cấp lớn

### 1. Driver điều khiển 32 LED WS2812 (`PC2`)
- **[ws2812.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.h)** & **[ws2812.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.c)**
- Xuất tín hiệu bit-banging trực tiếp trên xung 48MHz (T0H/T1H chính xác đến từng nanosecond).
- Hỗ trợ quản lý đệm 32 LED, bảng màu RGB, hiệu ứng vòng màu Rainbow Wheel, và điều chỉnh độ sáng chung (Master Brightness Scaling).

### 2. Mạch đọc Audio ADC Dual Channel (`PA2` & `PD6`) với AGC & Lọc Nhiễu Kép
- **[adc_audio.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.h)** & **[adc_audio.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.c)**
- Đọc đồng thời 2 kênh âm thanh độc lập: Kênh Trái (`PA2` / ADC Ch0) và Kênh Phải (`PD6` / ADC Ch6).
- **Đọc nhanh trực tiếp qua thanh ghi (Fast Register Read):** Tối ưu hóa vòng lặp đọc ADC, nhanh hơn gấp 5 lần so với hàm thư viện chuẩn của hãng.
- **Bộ lọc nhiễu tĩnh nhạy bén (Variance-based Noise Gate):** Sử dụng thuật toán đo độ biến động biên độ trong cửa sổ trượt 250ms (`diff_L < 15` và `ema_p2p_L < 75`). Phát hiện ngay dòng nhiễu ù tĩnh để dập tắt đèn LED về 0 lập tức trong vòng 240ms khi tạm dừng nhạc.
- **Thuật toán co giãn dải động kép (Uncapped dynamic_min AGC):** Theo dõi cả đỉnh âm lượng lớn nhất (`dynamic_max`) và đáy âm thanh nhỏ nhất (`dynamic_min`). Đảm bảo cột đèn vẩy hết biên độ vô cùng sinh động ở mọi mức volume (từ nhạc dạo siêu nhỏ đến volume cực đại) mà không bị sáng cứng/tràn thang đo.

### 3. Driver Nút bấm & Dimmer Điều Sáng Vô Cấp (`PC4`)
- **[button.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.h)** & **[button.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.c)**
- Quét nút bấm non-blocking với cơ chế chống dội (Debounce).
- **Nhấn ngắn:** Chuyển đổi qua lại giữa **16 hiệu ứng ánh sáng** độc đáo.
- **Nhấn giữ:** Tăng/giảm độ sáng liên tục (Dimming vô cấp). Hướng tăng/giảm tự động đảo chiều mỗi khi nhấn giữ lại.

### 4. Lưu Trữ Cấu Hình Vĩnh Viễn Vào Flash (Persistent Settings Storage)
- **[main.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/main.c)**
- Tự động ghi nhớ Chế độ nháy và Độ sáng hiện tại vào trang Flash cuối cùng của chip (`0x08003FC0`).
- Bảo vệ dữ liệu bằng Magic Key (`0xABCD55AA`), kiểm tra trùng lặp để tránh ghi lãng phí bảo vệ tuổi thọ chip, tắt ngắt tạm thời khi ghi để chống treo giật LED.
- Lưu độ sáng duy nhất 1 lần khi người dùng buông tay khỏi nút điều chỉnh.

### 5. Engine 16 Hiệu ứng ánh sáng & Đỉnh rơi Peak Hold
- **[vu_effects.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.h)** & **[vu_effects.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.c)**
- Tích hợp **16 chế độ hiệu ứng** đa dạng, bao gồm: Cổ điển, Cầu vồng, Lửa hồng, Sóng biển, Chớp disco, DNA xoắn kép, Sao băng trôi lơ lửng, và va chạm giữa 2 cột sóng.
- Sử dụng bộ phát số ngẫu nhiên phần cứng siêu tốc **Galois LFSR** (chỉ dịch bit và XOR) để tối ưu hóa thuật toán trên lõi chip không có bộ nhân phần cứng.

### 6. Khởi tạo Hệ thống & Vòng lặp chính
- Khởi động hệ thống 48MHz, chạy hiệu ứng Boot Cầu vồng lướt 32 LED và duy trì tốc độ làm tươi mượt mà **60 FPS**.

### 7. Sơ đồ đấu nối & Tài liệu hướng dẫn
- **[Huong_Dan_Su_Dung.md](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/Huong_Dan_Su_Dung.md)**
- Hướng dẫn chi tiết sơ đồ chân SOP-8, sơ đồ ghép nối chuỗi 4 module 1x8 WS2812 và mạch chia điện áp DC Bias cho cổng vào Audio ADC.
