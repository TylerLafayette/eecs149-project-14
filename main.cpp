#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>

#define MPU9250_ADDRESS 0x68 // I2C address of the MPU-9250

// MPU-9250 Register Addresses
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define PWR_MGMT_1 0x6B

// Scale factors (assuming default settings)
#define ACCEL_SCALE 16384.0 // for ±2g sensitivity
#define GYRO_SCALE 131.0    // for ±250°/s sensitivity

// Function to read a 16-bit value from two 8-bit registers
// int read_word_2c(int fd, int reg) {
//   int high = wiringPiI2CReadReg8(fd, reg);
//   int low = wiringPiI2CReadReg8(fd, reg + 1);
//   int value = (high << 8) + low;
//   if (value >= 0x8000) {
//     value = -(65536 - value);
//   }
//   return value;
// }

// void read_accel_gyro(int fd, float *accel_data, float *gyro_data) {
//   // Read accelerometer data
//   accel_data[0] = read_word_2c(fd, ACCEL_XOUT_H) / ACCEL_SCALE;
//   accel_data[1] = read_word_2c(fd, ACCEL_XOUT_H + 2) / ACCEL_SCALE;
//   accel_data[2] = read_word_2c(fd, ACCEL_XOUT_H + 4) / ACCEL_SCALE;
//
//   // Read gyroscope data
//   gyro_data[0] = read_word_2c(fd, GYRO_XOUT_H) / GYRO_SCALE;
//   gyro_data[1] = read_word_2c(fd, GYRO_XOUT_H + 2) / GYRO_SCALE;
//   gyro_data[2] = read_word_2c(fd, GYRO_XOUT_H + 4) / GYRO_SCALE;
// }

// int main() {
//   // Initialize the I2C interface
//   int fd = wiringPiI2CSetup(MPU9250_ADDRESS);
//   if (fd == -1) {
//     printf("Failed to initialize I2C.\n");
//     return -1;
//   }
//
//   // Wake up the MPU-9250 as it starts in sleep mode
//   wiringPiI2CWriteReg8(fd, PWR_MGMT_1, 0);
//
//   float accel_data[3];
//   float gyro_data[3];
//
//   while (1) {
//     // Read data from the IMU
//     read_accel_gyro(fd, accel_data, gyro_data);
//
//     // Display the data
//     printf("Accel X: %.2f g, Y: %.2f g, Z: %.2f g\n", accel_data[0],
//            accel_data[1], accel_data[2]);
//     printf("Gyro X: %.2f °/s, Y: %.2f °/s, Z: %.2f °/s\n", gyro_data[0],
//            gyro_data[1], gyro_data[2]);
//
//     // Sleep for a bit to limit output rate
//     usleep(100000); // 100 ms
//   }
//
//   return 0;
// }

int main() {
  // Open the default camera (usually 0 is the default camera ID for a USB
  // webcam)
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    printf("Error: Could not open the camera.\n");
    return -1;
  }

  // Set the camera resolution (optional)
  cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

  // Capture a single frame
  cv::Mat frame;
  cap >> frame; // Read the frame into the matrix
  if (frame.empty()) {
    printf("Error: Could not capture an image.\n");
    return -1;
  }

  // Save the captured image
  if (cv::imwrite("captured_image.jpg", frame)) {
    printf("Image captured and saved as 'captured_image.jpg'.\n");
  } else {
    printf("Error: Could not save the image.\n");
    return -1;
  }

  // Release the camera
  cap.release();
  return 0;
}
