#include "buzzer_driver.h"

constexpr uint8_t BUZZER_PIN = 5;      // <-- Change if required
constexpr uint8_t BUZZER_CHANNEL = 0;  // <-- Change if required

constexpr uint16_t PWM_FREQ = 2000;
constexpr uint8_t PWM_RESOLUTION = 8;

struct Tone
{
    uint16_t frequency;
    uint16_t duration;
};

static BuzzerPattern currentPattern = BuzzerPattern::NONE;

static bool playing = false;

static uint32_t previousMillis = 0;

static uint8_t toneIndex = 0;


/******************************************************
 * Tone Tables
 ******************************************************/

static const Tone startupPattern[] =
{
    {1000,120},
    {0,80},
    {1500,120},
};

static const Tone successPattern[] =
{
    {1800,80},
    {0,40},
    {1800,80},
};

static const Tone warningPattern[] =
{
    {1000,200},
    {0,200},
    {1000,200},
    {0,400},
};

static const Tone criticalPattern[] =
{
    {2500,100},
    {0,80},
    {2500,100},
    {0,80},
    {2500,100},
    {0,500},
};

static const Tone errorPattern[] =
{
    {500,250},
    {0,120},
    {500,250},
    {0,120},
    {500,250},
};


/******************************************************
 * Helpers
 ******************************************************/

static const Tone* getPattern(BuzzerPattern pattern, uint8_t &length)
{
    switch(pattern)
    {
        case BuzzerPattern::STARTUP:
            length = sizeof(startupPattern)/sizeof(Tone);
            return startupPattern;

        case BuzzerPattern::SUCCESS:
            length = sizeof(successPattern)/sizeof(Tone);
            return successPattern;

        case BuzzerPattern::WARNING:
            length = sizeof(warningPattern)/sizeof(Tone);
            return warningPattern;

        case BuzzerPattern::CRITICAL:
            length = sizeof(criticalPattern)/sizeof(Tone);
            return criticalPattern;

        case BuzzerPattern::ERROR:
            length = sizeof(errorPattern)/sizeof(Tone);
            return errorPattern;

        default:
            length = 0;
            return nullptr;
    }
}


/******************************************************
 * Driver
 ******************************************************/

bool Buzzer_begin(void)
{
    ledcSetup(BUZZER_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    ledcWrite(BUZZER_CHANNEL, 0);

    return true;
}


void Buzzer_play(BuzzerPattern pattern)
{
    currentPattern = pattern;

    toneIndex = 0;

    previousMillis = millis();

    playing = true;
}


void Buzzer_stop(void)
{
    ledcWrite(BUZZER_CHANNEL, 0);

    playing = false;

    currentPattern = BuzzerPattern::NONE;
}


bool Buzzer_isPlaying(void)
{
    return playing;
}


void Buzzer_update(void)
{
    if(!playing)
        return;

    uint8_t length;

    const Tone* pattern = getPattern(currentPattern,length);

    if(pattern==nullptr)
        return;

    if(millis()-previousMillis < pattern[toneIndex].duration)
        return;

    previousMillis = millis();

    if(pattern[toneIndex].frequency==0)
    {
        ledcWrite(BUZZER_CHANNEL, 0);
    }
    else
    {
        ledcWriteTone(BUZZER_CHANNEL,
                      pattern[toneIndex].frequency);
    }

    toneIndex++;

    if(toneIndex>=length)
    {
        if(currentPattern==BuzzerPattern::WARNING ||
           currentPattern==BuzzerPattern::CRITICAL)
        {
            toneIndex=0;
        }
        else
        {
            Buzzer_stop();
        }
    }
}