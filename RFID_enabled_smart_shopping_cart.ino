#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22  // RST pin for ESP32 WROOM-32
#define SS_PIN 5    // SS (SDA) pin

MFRC522 mfrc522(SS_PIN, RST_PIN);

struct Product {
    const char* name;
    float price;
};

// Mapping four RFID cards to products
const struct {
    const byte uid[4];
    Product product;
} productMap[] = {
    {{0x7B, 0xAE, 0xE9, 0x0A}, {"Surf Excel", 130.0}},
    {{0x46, 0x03, 0x3F, 0xF8}, {"Cornflakes", 50.0}},
    {{0x96, 0xD2, 0x38, 0xF8}, {"Milk", 40.0}},
    {{0x79, 0x0E, 0x8D, 0xBB}, {"Peanut Butter", 200.0}}
};

// Store scanned product UIDs
const int MAX_ITEMS = 10;  // Maximum number of scanned items
byte scannedUIDs[MAX_ITEMS][4]; 
int scannedCount = 0;
float totalPrice = 0.0;

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Ready to scan RFID tags...");
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        delay(100);
        return;
    }

    Product scannedProduct = {"Unknown", 0.0};
    bool alreadyScanned = false;
    int index = -1;

    // Check if scanned UID matches a stored product
    for (int i = 0; i < sizeof(productMap) / sizeof(productMap[0]); i++) {
        if (memcmp(mfrc522.uid.uidByte, productMap[i].uid, 4) == 0) {
            scannedProduct = productMap[i].product;
            break;
        }
    }

    // Check if product is already scanned
    for (int i = 0; i < scannedCount; i++) {
        if (memcmp(scannedUIDs[i], mfrc522.uid.uidByte, 4) == 0) {
            alreadyScanned = true;
            index = i;
            break;
        }
    }

    if (scannedProduct.price > 0.0) {
        if (alreadyScanned) {
            // Remove the product if it's scanned again
            totalPrice -= scannedProduct.price;
            Serial.print("Removed: ");
            Serial.println(scannedProduct.name);

            // Shift array left to remove the scanned product
            for (int i = index; i < scannedCount - 1; i++) {
                memcpy(scannedUIDs[i], scannedUIDs[i + 1], 4);
            }
            scannedCount--;
        } else {
            // Add the product if it's scanned for the first time
            if (scannedCount < MAX_ITEMS) {
                memcpy(scannedUIDs[scannedCount], mfrc522.uid.uidByte, 4);
                scannedCount++;
                totalPrice += scannedProduct.price;
                Serial.print("Added: ");
                Serial.println(scannedProduct.name);
            } else {
                Serial.println("Cart full! Cannot add more items.");
            }
        }

        Serial.print("Total Cost: ₹");
        Serial.println(totalPrice, 2);
    } else {
        Serial.println("Unknown card detected!");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1000);  // Avoid continuous scanning issues
}
