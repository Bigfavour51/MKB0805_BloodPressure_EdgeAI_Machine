#ifndef ANOMALY_AI_H
#define ANOMALY_AI_H

#include <Arduino.h>

// Initializes the AI engine
void EdgeAI_Init();

// Runs the Autoencoder Forward Pass and returns the Reconstruction Error (Anomaly Score)
float EdgeAI_GetAnomalyScore(float sys, float dia, float hr, float spo2);

#endif