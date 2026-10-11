import pandas as pd
import numpy as np
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# ================= 1. CẤU HÌNH INFLUXDB =================
INFLUX_TOKEN = "ĐIỀN_TOKEN_CỦA_BẠN_VÀO_ĐÂY"
INFLUX_ORG = "PTIT"
INFLUX_URL = "http://localhost:8086"
BUCKET_RAW = "sensor_data"
BUCKET_CLEAN = "sensor_data_clean"

# Khởi tạo kết nối
client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
query_api = client.query_api()
write_api = client.write_api(write_options=SYNCHRONOUS)

print("1. Đang lấy dữ liệu thô từ InfluxDB...")
# Lấy dữ liệu trong 1 giờ qua
query = f'''
    from(bucket: "{BUCKET_RAW}")
    |> range(start: -1h)
    |> filter(fn: (r) => r._measurement == "environment")
    |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
'''

try:
    df = query_api.query_data_frame(query)
    
    if df.empty:
        print("Không có dữ liệu nào trong 1 giờ qua để xử lý!")
    else:
        # Xóa các cột hệ thống dư thừa của InfluxDB
        columns_to_drop = ['result', 'table', '_start', '_stop', '_measurement', 'device']
        df = df.drop(columns=[col for col in columns_to_drop if col in df.columns], errors='ignore')
        
        # Đặt thời gian làm index cho Pandas
        df.set_index('_time', inplace=True)
        df.index = pd.to_datetime(df.index)

        print("2. Đang tiến hành tiền xử lý dữ liệu...")
        
        # 2.1 Xử lý giá trị thiếu (Linear Interpolation)
        df = df.interpolate(method='linear')

        # 2.2 Phát hiện và loại bỏ Outlier (Z-score > 3) cho Nhiệt độ
        if 'temperature' in df.columns:
            z_scores = np.abs((df['temperature'] - df['temperature'].mean()) / df['temperature'].std())
            # Đánh dấu các điểm nhiễu là NaN
            df.loc[z_scores > 3, 'temperature'] = np.nan 
            # Điền lại các điểm nhiễu bằng trung bình trượt của 5 giá trị gần nhất
            df['temperature'] = df['temperature'].fillna(df['temperature'].rolling(5, min_periods=1).mean())

        # 2.3 Resampling (Hạ tần số lấy mẫu xuống trung bình mỗi 1 phút)
        df_resampled = df.resample('1T').mean()

        # 2.4 Feature Engineering (Tạo đặc trưng mới: Nhiệt độ trung bình trượt 10 phút)
        if 'temperature' in df_res
