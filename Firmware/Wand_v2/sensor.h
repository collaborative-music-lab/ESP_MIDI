class SensorChannel {
private:
    float raw = 0.0;
    float smoothed = 0.0;
    float prevSmoothed = 0.0;
    float derivative = 0.0;
    float integrated = 0.0;
    float scale = 1.0;

    float smoothingAlpha = 0.1;    // Low-pass smoothing factor
    float integrationDecay = 1.0;  // Leak factor (1.0 = no leak)

    unsigned long lastUpdate = 0;

public:
    uint8_t enable = 1;
    // Call once per sample
    void update(float newValue) {
        unsigned long now = millis();
        float dt = (lastUpdate > 0) ? (now - lastUpdate) / 1000.0 : 0.01;
        lastUpdate = now;

        raw = newValue;

        // Low-pass filter
        prevSmoothed = smoothed;
        smoothed = smoothingAlpha * raw + (1.0 - smoothingAlpha) * smoothed;

        // Derivative
        derivative = (smoothed - prevSmoothed) / dt;

        // Leaky integrator
        integrated = (integrated + smoothed * dt) * integrationDecay;
    }

    // Reset integrated value
    void resetIntegrated() {
        integrated = 0.0;
    }

    // Setters
    void setSmoothing(float alpha) {
        smoothingAlpha = constrain(alpha, 0.0, 1.0);
    }

    void setIntegrationDecay(float decay) {
        integrationDecay = constrain(decay, 0.0, 1.0);
    }

    void setScale(float s) {
        scale = s;
    }

    void setScaleFromCC(uint8_t ccVal) {
        scale = map(ccVal, 0, 127, 0, 200) / 100.0;
    }

    // Accessors
    float getRaw() const { return raw; }
    float getSmoothed() const { return smoothed; }
    float getDerivative() const { return derivative; }
    float getIntegrated() const { return integrated; }

    // Scaled versions
    float getScaledSmoothed() const { return smoothed * scale; }
    float getScaledDerivative() const { return derivative * scale; }
    float getScaledIntegrated() const { return integrated * scale; }
};

struct SensorTriple {
    SensorChannel x;
    SensorChannel y;
    SensorChannel z;

    void update(float valX, float valY, float valZ) {
        x.update(valX);
        y.update(valY);
        z.update(valZ);
    }

    void resetIntegrated() {
        x.resetIntegrated();
        y.resetIntegrated();
        z.resetIntegrated();
    }

    void setScale(float sx, float sy, float sz) {
        x.setScale(sx);
        y.setScale(sy);
        z.setScale(sz);
    }

    void setSmoothing(float alpha) {
        x.setSmoothing(alpha);
        y.setSmoothing(alpha);
        z.setSmoothing(alpha);
    }

    void setIntegrationDecay(float decay) {
        x.setIntegrationDecay(decay);
        y.setIntegrationDecay(decay);
        z.setIntegrationDecay(decay);
    }
};