# SƠ ĐỒ HƯỚNG DẪN LẮP RÁP & SỬ DỤNG STEREO VU LED METER (CH32V003J4M6)

Dự án VU LED Stereo 2 kênh sử dụng vi điều khiển **CH32V003J4M6 (SOP-8)** điều khiển chuỗi **32 LED WS2812** (4 module 1x8 ghép nối tiếp) với **17 chế độ hiệu ứng** ánh sáng độc đáo.

---

## 1. Sơ đồ đấu nối phần cứng (SOP-8 Pinout)

```text
                     CH32V003J4M6 (SOP-8)
                          ┌──────┐
   Audio In Kênh Phải ──1─┤PD6   │PD4├─8── SWIO (WCH-LinkE Debug Pin)
            GND Chung ──2─┤GND   │PC4├─7── Button In (Mode Switch -> GND)
Audio In Kênh Trái/SWCLK ──3─┤PA2   │PC2├─6── WS2812 Data Out (DIN)
             VCC 3.3V ──4─┤VCC   │PC1├─5── Extra / Unused (formerly Audio Left)
                          └──────┘
```

> [!WARNING]
> **Cảnh báo về chân nạp SWCLK (PA2):**
> Chân 3 (`PA2`) vừa làm cổng nhận âm thanh Kênh Trái vừa là chân xung nhịp nạp `SWCLK`. Do mã nguồn cấu hình chân này thành Analog Input, cổng nạp sẽ bị khóa sau khi chip khởi động. 
> Khi cần nạp code mới, bạn **bắt buộc phải thực hiện thao tác kích nguồn** (rút VCC, bấm nạp trên MRS, rồi cắm nhanh VCC lại) để mạch nạp có thể kết nối trước khi chip khóa chân.

### Bảng chi tiết đấu nối:
1. **Nguồn điện:**
   - Cấp nguồn **5V / 2A** cho 4 module WS2812.
   - Cấp nguồn **3.3V** cho chip CH32V003J4M6 (qua IC nguồn AMS1117-3.3V hoặc hạ áp).
   - **LƯU Ý:** Phải nối chung chân **GND** của nguồn 5V, 3.3V, mạch âm thanh và MCU.

2. **Kết nối 4 Module LED WS2812 (Tổng 32 LED):**
   - Chân `PC2` (Pin 6 MCU) $\rightarrow$ Đấu vào chân **DIN** của Module 1.
   - Chân **DOUT** Module 1 $\rightarrow$ **DIN** Module 2.
   - Chân **DOUT** Module 2 $\rightarrow$ **DIN** Module 3.
   - Chân **DOUT** Module 3 $\rightarrow$ **DIN** Module 4.
   - *(LED 0 - 15 là Kênh Trái, LED 16 - 31 là Kênh Phải).*

3. **Kết nối Mạch Audio Input (Kênh Trái PA2 & Kênh Phải PD6):**
   Mỗi kênh Audio cần 1 mạch phân áp DC bias đơn giản để ADC đo tín hiệu xoay chiều:
   ```text
   Tín hiệu Audio (L/R) ────┤├────┬─────────> Chân PA2 / PD6 (MCU)
                        1uF Tụ   │
                                ┌┴┐ 10k
                                │ │ 
                                └┬┘
             VCC 3.3V ──[10k]────┼────[10k]── GND
   ```

4. **Nút nhấn chuyển hiệu ứng (`PC4`):**
   - 1 chân nút bấm đấu vào `PC4` (Pin 7).
   - Chân còn lại của nút bấm đấu xuống **GND**.

---

## 2. Thao tác điều khiển Nút bấm (`PC4`)

* **Nhấn ngắn (< 0.6 giây):** Chuyển qua lại giữa **17 chế độ hiệu ứng**:
  1. `Mode 0`: Classic Green-Yellow-Red Gradient + Đỉnh rơi Peak Dot màu trắng.
  2. `Mode 1`: Mưa sao băng (Meteor Shower) đầu trắng đuôi lửa lướt lơ lửng.
  3. `Mode 2`: Rainbow Dynamic Gradient + Peak Dot trắng.
  4. `Mode 3`: Center-Out Symmetric Meter (Tín hiệu tỏa từ giữa ra 2 biên).
  5. `Mode 4`: Fire / Heat Map Effect (Ngọn lửa bùng phát theo Volume).
  6. `Mode 5`: Ocean Cyan & Blue Wave (Tông màu xanh đại dương dịu mắt).
  7. `Mode 6`: Solid Color Pulse (Phong cách tối giản nhấp nháy theo âm lượng).
  8. `Mode 7`: Club Strobe (Nháy toàn bộ dải LED chớp động cực mạnh).
  9. `Mode 8`: Split Color (Chia đôi màu sắc đối xứng sắc nét).
  10. `Mode 9`: Out-In Symmetric (Tín hiệu dồn từ 2 rìa biên vào giữa).
  11. `Mode 10`: Green Column + Red Peak (Cột xanh lá với điểm đỉnh màu đỏ nổi bật).
  12. `Mode 11`: Rainbow Flow Animation (Hiệu ứng sóng cầu vồng trôi liên tục).
  13. `Mode 12`: Interlaced Flow (Hiệu ứng sóng kép DNA đan chéo).
  14. `Mode 13`: Fading Glow Trails (Hiệu ứng vẩy tia lửa để lại vệt sáng mờ dần).
  15. `Mode 14`: Collision Beat (Cột sóng hai bên va chạm nhau ở giữa tạo tia sáng).
  16. `Mode 15`: Level Alert (Cột đổi màu toàn bộ dựa trên độ cao âm lượng).
  17. `Mode 16`: Rainbow Accordion (Sóng cầu vồng co giãn rộng hẹp theo Volume).

* **Nhấn giữ (> 0.5 giây):** Tăng hoặc giảm độ sáng liên tục (Dimming vô cấp). Hướng tăng/giảm sẽ tự động đảo chiều ở mỗi lần giữ tiếp theo. Mức độ sáng được tự động lưu vĩnh viễn vào bộ nhớ Flash ngay sau khi nhả nút.

---

## 3. Cấu trúc thư mục Code trong dự án

- [main.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/main.c): Vòng lặp chính, khởi tạo và điều phối các module.
- [ws2812.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.h) & [ws2812.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/ws2812.c): Driver bit-banging tốc độ cao xuất dữ liệu cho 32 LED qua `PC2`.
- [adc_audio.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.h) & [adc_audio.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/adc_audio.c): Lấy mẫu tín hiệu âm thanh Stereo 2 kênh `PA2` (Trái) & `PD6` (Phải) với bộ tự động thích ứng dải động (Auto Gain Control - AGC).
- [button.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.h) & [button.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/button.c): Quét nút bấm chống dội trên `PC4`.
- [vu_effects.h](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.h) & [vu_effects.c](file:///d:/Projects_2026/VU_LED_W2812/CH32V003J4M/User/vu_effects.c): Engine quản lý 17 hiệu ứng và vật lý đỉnh rơi (Peak Hold & Gravity Decay).
