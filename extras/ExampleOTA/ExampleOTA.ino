
void setup()
{
    Serial.begin(115200);
    for(const auto timeout = 2500u; !Serial && millis() < timeout; yield());

    delay(2500);
    Serial.println();
    Serial.println();
    Serial.println("***** ***** ***** ***** *****");
    Serial.println("After USB OTA");
    Serial.print("Compilation Datetime: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);
    Serial.println("***** ***** ***** ***** *****");
}

void loop()
{
    Serial.print("[");
    Serial.print(millis());
    Serial.println("] Hello, World!");
    delay(1000);
}
