#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

class BuzzerController {
public:
    BuzzerController(int buzzerPin);
    void beep(int frequency, int duration);

private:
    int buzzerPin;
};

#endif