#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

class LEDController {
public:
    LEDController(int greenPin, int bluePin, int redPin);
    void setGreen();
    void setRed();
    void reset();

private:
    int greenPin, bluePin, redPin;
};

#endif