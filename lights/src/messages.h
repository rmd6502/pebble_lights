typedef struct Light
{
    uint16_t red, green, blue;
    char name[32];
} Light;

typedef void (*GetLightCallback)(Light *light);
void message_init();
uint8_t setLight(Light *light);
uint8_t getLight(Light *light, GetLightCallback callback);
