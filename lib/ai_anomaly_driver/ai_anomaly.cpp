#include "ai_anomaly.h"
#include <math.h>

// Neural Network Architecture: 4 Inputs -> 2 Hidden -> 4 Outputs
#define INPUT_NODES 4
#define HIDDEN_NODES 2
#define OUTPUT_NODES 4

// Pre-trained weights mimicking a healthy baseline (Normally generated via Python/Keras)
const float weights_input_hidden[INPUT_NODES][HIDDEN_NODES] = {
    {0.50f, -0.20f}, // SYS weights
    {0.45f, -0.15f}, // DIA weights
    {0.30f, 0.10f},  // HR weights
    {-0.10f, 0.80f}  // SpO2 weights
};

const float weights_hidden_output[HIDDEN_NODES][OUTPUT_NODES] = {
    {0.50f, 0.45f, 0.30f, -0.10f},
    {-0.20f, -0.15f, 0.10f, 0.80f}
};

const float bias_hidden[HIDDEN_NODES] = {0.1f, 0.1f};
const float bias_output[OUTPUT_NODES] = {0.05f, 0.05f, 0.05f, 0.1f};

// Activation Function (ReLU)
float relu(float x) {
    return (x > 0.0f) ? x : 0.0f;
}

void EdgeAI_Init() {
    Serial.println("[Edge AI] Autoencoder Neural Network Initialized.");
    Serial.println("[Edge AI] Architecture: 4 -> 2 -> 4");
}

float EdgeAI_GetAnomalyScore(float sys, float dia, float hr, float spo2) {
    // 1. Min-Max Normalization (Scale inputs to 0.0 - 1.0)
    // Limits: SYS(80-180), DIA(50-120), HR(40-140), SpO2(80-100)
    float input[INPUT_NODES];
    input[0] = constrain((sys - 80.0f) / 100.0f, 0.0f, 1.0f);
    input[1] = constrain((dia - 50.0f) / 70.0f,  0.0f, 1.0f);
    input[2] = constrain((hr - 40.0f) / 100.0f,  0.0f, 1.0f);
    input[3] = constrain((spo2 - 80.0f) / 20.0f, 0.0f, 1.0f);

    // 2. Forward Pass: Encoder (Input -> Hidden)
    float hidden[HIDDEN_NODES] = {0};
    for (int i = 0; i < HIDDEN_NODES; i++) {
        hidden[i] = bias_hidden[i];
        for (int j = 0; j < INPUT_NODES; j++) {
            hidden[i] += input[j] * weights_input_hidden[j][i];
        }
        hidden[i] = relu(hidden[i]); // Apply Activation
    }

    // 3. Forward Pass: Decoder (Hidden -> Output)
    float output[OUTPUT_NODES] = {0};
    for (int i = 0; i < OUTPUT_NODES; i++) {
        output[i] = bias_output[i];
        for (int j = 0; j < HIDDEN_NODES; j++) {
            output[i] += hidden[j] * weights_hidden_output[j][i];
        }
        output[i] = relu(output[i]); // Apply Activation
    }

    // 4. Calculate Mean Squared Error (MSE) / Reconstruction Error
    float mse = 0.0f;
    for (int i = 0; i < INPUT_NODES; i++) {
        float error = input[i] - output[i];
        mse += (error * error);
    }
    mse = mse / INPUT_NODES;

    // Amplify MSE slightly for easier thresholding in the demo
    return mse * 100.0f; 
}