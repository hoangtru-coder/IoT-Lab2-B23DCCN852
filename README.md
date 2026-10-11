# BÀI THỰC HÀNH SỐ 2: THU THẬP, LƯU TRỮ VÀ TIỀN XỬ LÝ DỮ LIỆU IOT

**Thông tin sinh viên:**
- **Họ và tên:** Hoàng Phúc Vũ Trụ
- **Mã sinh viên:** B23DCCN852
- **Lớp/Chuyên ngành:** Hệ thống thông tin - PTIT

---

## 1. Cấu trúc thư mục (Repository Structure)
- `firmware_esp32.ino`: Mã nguồn C++ chạy trên vi điều khiển ESP32 (giả lập bằng Wokwi), chịu trách nhiệm đọc dữ liệu cảm biến DHT22, HC-SR04 và gửi lên MQTT Broker.
- `thu_thap_mqtt.py`: Script Python đóng vai trò MQTT Subscriber, nhận dữ liệu JSON từ thiết bị và lưu trực tiếp vào cơ sở dữ liệu InfluxDB (Raw Data).
- `tien_xu_ly.py`: Script Python đọc dữ liệu từ InfluxDB, sử dụng Pandas để làm sạch, lọc nhiễu, điền khuyết và nội suy dữ liệu trước khi lưu lại vào bucket mới.
- `requirements.txt`: Danh sách các thư viện Python cần thiết để chạy hệ thống.

---

## 2. Hướng dẫn cài đặt (Installation)

**Yêu cầu hệ thống:**
- Python 3.8+
- InfluxDB v2.x đang chạy ở Localhost (cổng mặc định 8086)

**Cài đặt thư viện:**
Mở Terminal / Command Prompt và chạy lệnh sau để cài đặt các thư viện phụ thuộc:
```bash
pip install -r requirements.txt
