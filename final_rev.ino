#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Adafruit_TFLite.h"
#include "model_data.h"  // Include your TensorFlow Lite model header file

// Define MPU6050 objects for two sensors
MPU6050 mpu1;
MPU6050 mpu2(0x69); // Initialize second MPU6050 sensor with I2C address 0x69

// Define TensorFlow Lite globals
constexpr int kTensorArenaSize = 3 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroErrorReporter micro_error_reporter;
const tflite::Model* model;
tflite::MicroInterpreter* interpreter;
TfLiteTensor* model_input;
TfLiteTensor* model_output;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Initialize first MPU6050 sensor
    Wire.begin();
    mpu1.initialize();

    // Initialize second MPU6050 sensor
    mpu2.initialize();

    // Initialize TensorFlow Lite model and interpreter
    model = tflite::GetModel(model_data);
    static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
    interpreter = new tflite::MicroInterpreter(model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);
    interpreter->AllocateTensors();
    model_input = interpreter->input(0);
    model_output = interpreter->output(0);
}

void loop() {
    // Read accelerometer data from first MPU6050 sensor
    int16_t ax1, ay1, az1;
    mpu1.getAcceleration(&ax1, &ay1, &az1);

    // Read accelerometer data from second MPU6050 sensor
    int16_t ax2, ay2, az2;
    mpu2.getAcceleration(&ax2, &ay2, &az2);

    // Set input tensor with sensor data
    model_input->data.f[0] = static_cast<float>(ax1);
    model_input->data.f[1] = static_cast<float>(ay1);
    model_input->data.f[2] = static_cast<float>(az1);
    model_input->data.f[3] = static_cast<float>(ax2);
    model_input->data.f[4] = static_cast<float>(ay2);
    model_input->data.f[5] = static_cast<float>(az2);

    // Run inference
    interpreter->Invoke();

    // Get output tensor
    float output_values[7];
    for (int i = 0; i < 7; i++) {
        output_values[i] = model_output->data.f[i];
    }

    // Find the index of the highest value in the output tensor
    int predicted_class = 0;
    for (int i = 1; i < 7; i++) {
      // serial.println(output_values[i]);
        if (output_values[i] > output_values[predicted_class]) {
            predicted_class = i;
        }
    }
    
    // Process output as needed (e.g., print or use it in your application)
    Serial.print("Predicted class: ");
    switch (predicted_class) {
        case 0:
            Serial.println("Walking");
            break;
        case 1:
            Serial.println("Shuffling");
            break;
        case 2:
            Serial.println("Ascending stairs");
            break;
        case 3:
            Serial.println("Descending Stairs");
            break;
        case 4:
            Serial.println("Standing");
            break;
        case 5:
            Serial.println("Sitting");
            break;
        case 6:
            Serial.println("Lying");
            break;
        default:
            Serial.println("Unknown class");
            break;
    }

    // Delay for a short period
    delay(100);
}


//using standarisation
/*#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Adafruit_TFLite.h"
#include "model_data.h"  // Include your TensorFlow Lite model header file

// Define MPU6050 objects for two sensors
MPU6050 mpu1;
MPU6050 mpu2(0x69); // Initialize second MPU6050 sensor with I2C address 0x69

// Define TensorFlow Lite globals
constexpr int kTensorArenaSize = 5 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
tflite::MicroErrorReporter micro_error_reporter;
const tflite::Model* model;
tflite::MicroInterpreter* interpreter;
TfLiteTensor* model_input;
TfLiteTensor* model_output;

// Precomputed mean and standard deviation values
float mean_values[] = { -0.868381, -0.031788, 0.022442, -0.676396, 0.008185, -0.385882 };
float stddev_values[] = { 0.275664, 0.155677, 0.427955, 0.559683, 0.270732, 0.508702 };

// Standardization function
float standardize(float value, float mean, float stddev) {
    return (value - mean) / stddev;
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Initialize first MPU6050 sensor
    Wire.begin();
    mpu1.initialize();

    // Initialize second MPU6050 sensor
    mpu2.initialize();

    // Initialize TensorFlow Lite model and interpreter
    model = tflite::GetModel(model_data);
    static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
    interpreter = new tflite::MicroInterpreter(model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);
    interpreter->AllocateTensors();
    model_input = interpreter->input(0);
    model_output = interpreter->output(0);
}

void loop() {
    // Read accelerometer data from first MPU6050 sensor
    int16_t ax1, ay1, az1;
    mpu1.getAcceleration(&ax1, &ay1, &az1);

    // Read accelerometer data from second MPU6050 sensor
    int16_t ax2, ay2, az2;
    mpu2.getAcceleration(&ax2, &ay2, &az2);

    // Standardize data
    float std_ax1 = standardize(ax1, mean_values[0], stddev_values[0]);
    float std_ay1 = standardize(ay1, mean_values[1], stddev_values[1]);
    float std_az1 = standardize(az1, mean_values[2], stddev_values[2]);

    float std_ax2 = standardize(ax2, mean_values[3], stddev_values[3]);
    float std_ay2 = standardize(ay2, mean_values[4], stddev_values[4]);
    float std_az2 = standardize(az2, mean_values[5], stddev_values[5]);

    // Set input tensor with standardized sensor data
    model_input->data.f[0] = std_ax1;
    model_input->data.f[1] = std_ay1;
    model_input->data.f[2] = std_az1;
    model_input->data.f[3] = std_ax2;
    model_input->data.f[4] = std_ay2;
    model_input->data.f[5] = std_az2;

    // Run inference
    interpreter->Invoke();

    // Get output tensor
    float output_values[7];
    for (int i = 0; i < 7; i++) {
        output_values[i] = model_output->data.f[i];
    }

    // Find the index of the highest value in the output tensor
    int predicted_class = 0;
    for (int i = 1; i < 7; i++) {
        if (output_values[i] > output_values[predicted_class]) {
            predicted_class = i;
        }
    }

    // Process output as needed (e.g., print or use it in your application)
    Serial.print("Predicted class: ");
    switch (predicted_class) {
        case 0:
            Serial.println("Class A");
            break;
        case 1:
            Serial.println("Class B");
            break;
        case 2:
            Serial.println("Class C");
            break;
        case 3:
            Serial.println("Class D");
            break;
        case 4:
            Serial.println("Class E");
            break;
        case 5:
            Serial.println("Class F");
            break;
        case 6:
            Serial.println("Class G");
            break;
        default:
            Serial.println("Unknown class");
            break;
    }

    // Delay for a short period
    delay(100);
}*/